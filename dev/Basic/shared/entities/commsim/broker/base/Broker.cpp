//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "entities/commsim/Broker.hpp"

#include <sstream>
#include <boost/assign/list_of.hpp>
#include <json/json.h>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "workers/Worker.hpp"

#include "entities/commsim/broker/base/Common.hpp"
#include "entities/commsim/comm_support/AgentCommUtility.hpp"
#include "entities/commsim/connection/ConnectionServer.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/event/RegionsAndPathEventArgs.hpp"

#include "entities/commsim/wait/WaitForAndroidConnection.hpp"
#include "entities/commsim/wait/WaitForNS3Connection.hpp"
#include "entities/commsim/wait/WaitForAgentRegistration.hpp"

#include "event/SystemEvents.hpp"
#include "event/args/EventArgs.hpp"
#include "message/MessageBus.hpp"
#include "event/EventPublisher.hpp"

#include "geospatial/RoadRunnerRegion.hpp"


using namespace sim_mob;

std::map<std::string, sim_mob::Broker*> sim_mob::Broker::externalCommunicators;

sim_mob::Broker::Broker(const MutexStrategy& mtxStrat, int id, std::string commElement, std::string commMode) :
		Agent(mtxStrat, id), enabled(true) , commElement(commElement), commMode(commMode),
		brokerCanTickForward(false), numAgents(0)
{
	//Various Initializations
	connection.reset(new ConnectionServer(*this));
	/*m_messageReceiveCallback = boost::function<
		void(boost::shared_ptr<ConnectionHandler>, std::string)>(
		boost::bind(&Broker::messageReceiveCallback, this, _1, _2)
	);*/

//	configure();
}

sim_mob::Broker::~Broker()
{
}

void sim_mob::Broker::enable()
{
	enabled = true;
}

void sim_mob::Broker::disable()
{
	enabled = false;
}

bool sim_mob::Broker::isEnabled() const
{
	return enabled;
}

bool sim_mob::Broker::insertSendBuffer(boost::shared_ptr<sim_mob::ClientHandler> client,  const std::string& message)
{
	if(!(client && client->connHandle && client->isValid() && client->connHandle->isValid())) {
		return false;
	}

	boost::unique_lock<boost::mutex> lock(mutex_send_buffer);

	//Is this the first message received for this ClientHandler/destID pair?
	if (sendBuffer.find(client)==sendBuffer.end()) {
		OngoingSerialization& ongoing = sendBuffer[client];
		CommsimSerializer::serialize_begin(ongoing, client->clientId);
	}

	//Now just add it.
	CommsimSerializer::addGeneric(sendBuffer[client], message);
	return true;
}

void sim_mob::Broker::configure()
{
	//Only configure once.
	if (!configured_.check()) {
		return;
	}

	//Dispatch differently depending on whether we are using "android-ns3" or "android-only"
	//TODO: Find a more dynamic way of adding new clients.
	const std::string &client_type =
			ConfigManager::GetInstance().FullConfig().getCommSimMode(commElement);

	sim_mob::Worker::GetUpdatePublisher().subscribe(sim_mob::event::EVT_CORE_AGENT_UPDATED, 
                this, 
                &Broker::onAgentUpdate,
                (event::Context)sim_mob::event::CXT_CORE_AGENT_UPDATE);

	{
	BrokerPublisher* onlyLocationsPublisher = new BrokerPublisher();
	onlyLocationsPublisher->registerEvent(COMMEID_LOCATION);
	publishers.insert(std::make_pair(
		sim_mob::Services::SIMMOB_SRV_LOCATION,
		PublisherList::Value(onlyLocationsPublisher))
	);
	}

	//The Region publisher should be generally useful.
	{
	BrokerPublisher* regionPublisher = new BrokerPublisher();
	regionPublisher->registerEvent(COMMEID_REGIONS_AND_PATH);
	publishers.insert(std::make_pair(sim_mob::Services::SIMMOB_SRV_REGIONS_AND_PATH, PublisherList::Value(regionPublisher)));
	}

	//NS-3 has its own publishers
	if (client_type == "android-ns3") {
		BrokerPublisher* allLocationsPublisher = new BrokerPublisher();
		allLocationsPublisher->registerEvent(COMMEID_LOCATION);
		publishers.insert(std::make_pair(
			sim_mob::Services::SIMMOB_SRV_ALL_LOCATIONS,
			PublisherList::Value(allLocationsPublisher))
		);
	}

	{
	BrokerPublisher* timePublisher = new BrokerPublisher();
	timePublisher->registerEvent(COMMEID_TIME);
	publishers.insert(std::make_pair(
		sim_mob::Services::SIMMOB_SRV_TIME,
		PublisherList::Value(timePublisher))
	);
	}

	registrationPublisher.subscribe((event::EventId)comm::ANDROID_EMULATOR, this, &Broker::onClientRegister);

	if (client_type == "android-ns3") {
		//listen to publishers who announce registration of new clients...

		registrationPublisher.subscribe((event::EventId)comm::NS3_SIMULATOR, this, &Broker::onClientRegister);
	}

	bool useNs3 = false;
	if (client_type == "android-ns3") {
		useNs3 = true;
	} else if (client_type == "android-only") {
		useNs3 = false;
	} else { throw std::runtime_error("Unknown clientType in Broker."); }


	//Register handlers with "useNs3" flag. OpaqueReceive will throw an exception if it attempts to process a message and useNs3 is not set.
	handleLookup.addHandlerOverride("OPAQUE_SEND", new sim_mob::OpaqueSendHandler(useNs3));
	handleLookup.addHandlerOverride("OPAQUE_RECEIVE", new sim_mob::OpaqueReceiveHandler(useNs3));


	// wait for connection criteria for this broker
	clientBlockers.insert(std::make_pair(comm::ANDROID_EMULATOR,
			boost::shared_ptr<sim_mob::WaitForAndroidConnection>(new sim_mob::WaitForAndroidConnection(*this,MIN_CLIENTS))));


	if(client_type == "android-ns3") {
		clientBlockers.insert(std::make_pair(comm::NS3_SIMULATOR,
				boost::shared_ptr<WaitForNS3Connection>(new WaitForNS3Connection(*this))));
	}

	// wait for connection criteria for this broker
	agentBlockers.insert(
		std::make_pair(0,
			boost::shared_ptr<WaitForAgentRegistration>(
				new WaitForAgentRegistration(*this, MIN_AGENTS)))
	);
}



/**
 * the callback function called by connection handlers to ask
 * the Broker what to do when a message arrives(this method is to be called
 * after whoareyou, registration and connectionHandler creation....)
 * anyway, what is does is :extract messages from the input string
 * (remember that a message object carries a reference to its handler also(except for "CLIENT_MESSAGES_DONE" messages)
 *
 * NOTE: Be careful; this function can be called multiple times by different "threads". Make sure you are locking where required.
 */
void sim_mob::Broker::onMessageReceived(boost::shared_ptr<ConnectionHandler> cnnHandler, const BundleHeader& header, std::string input)
{
	//If the stream has closed, fail. (Otherwise, the simulation will freeze.)
	if(input.empty()){
		throw std::runtime_error("Empty packet; canceling simulation.");
	}

	//Let the CommsimSerializer handle the heavy lifting.
	MessageConglomerate conglom;
	if (!CommsimSerializer::deserialize(header, input, conglom)) {
		throw std::runtime_error("Broker couldn't parse packet.");
	}


	//We have to introspect a little bit, in order to find our CLIENT_MESSAGES_DONE and WHOAMI messages.
	bool res = false;
	for (int i=0; i<conglom.getCount(); i++) {
		if (NEW_BUNDLES) {
			throw std::runtime_error("parseAgentsInfo() for NEW_BUNDLES not yet supported.");
		} else {
			const Json::Value& jsMsg = conglom.getMessage(i);
			if (jsMsg.isMember("MESSAGE_TYPE") && jsMsg["MESSAGE_TYPE"] == "CLIENT_MESSAGES_DONE") {
				boost::unique_lock<boost::mutex> lock(mutex_clientDone);
				{
				boost::unique_lock<boost::mutex> lock(mutex_client_done_chk);
				std::map<boost::shared_ptr<sim_mob::ConnectionHandler>, ConnClientStatus>::iterator chkIt = clientDoneChecklist.find(cnnHandler);
				if (chkIt==clientDoneChecklist.end()) {
					throw std::runtime_error("Unexpected client/connection mapping.");
				}
				chkIt->second.done++;
				} //mutex_client_done_chk unlocks

				if (EnableDebugOutput) {
					Print() << "connection [" <<&(*cnnHandler) << "] DONE\n";
				}


				COND_VAR_CLIENT_DONE.notify_one();
			} else if (jsMsg.isMember("MESSAGE_TYPE") && jsMsg["MESSAGE_TYPE"] == "WHOAMI") {
				//Since we now have a WHOAMI request AND a valid ConnectionHandler, we can pend a registration request.
				//TODO: This can definitely be simplified; it was copied from WhoAreYouProtocol and Jsonparser.
				sim_mob::ClientRegistrationRequest candidate;
				if (!(jsMsg.isMember("ID") && jsMsg.isMember("TYPE") && jsMsg.isMember("token"))) {
					throw std::runtime_error("Can't access required fields.");
				}
				candidate.clientID = jsMsg["ID"].asString();
				candidate.client_type = jsMsg["TYPE"].asString();
				if (!jsMsg["REQUIRED_SERVICES"].isNull() && jsMsg["REQUIRED_SERVICES"].isArray()) {
					const Json::Value services = jsMsg["REQUIRED_SERVICES"];
					for (size_t index=0; index<services.size(); index++) {
						std::string type = services[int(index)].asString();
						candidate.requiredServices.insert(Services::GetServiceType(type));
					}
				}

				//Retrieve the token.
				std::string token = jsMsg["token"].asString();

				//What type is this?
				std::map<std::string, comm::ClientType>::const_iterator clientTypeIt = sim_mob::Services::ClientTypeMap.find(candidate.client_type);
				if (clientTypeIt == sim_mob::Services::ClientTypeMap.end()) {
					throw std::runtime_error("Client type is unknown; cannot re-assign.");
				}

				//Retrieve the connection associated with this token.
				boost::shared_ptr<ConnectionHandler> connHandle;
				{
					boost::unique_lock<boost::mutex> lock(mutex_WaitingWHOAMI);
					std::map<std::string, boost::shared_ptr<sim_mob::ConnectionHandler> >::const_iterator connHan = tokenConnectionLookup.find(token);
					if (connHan == tokenConnectionLookup.end()) {
						throw std::runtime_error("Unknown token; can't receive WHOAMI.");
					}
					connHandle = connHan->second;
				}
				if (!connHandle) {
					throw std::runtime_error("WHOAMI received, but no clients are in the waiting list.");
				}

				//At this point we need to check the Connection's clientType and set it if it is UNKNOWN.
				//If it is known, make sure it's the expected type.
				if (connHandle->getClientType() == comm::UNKNOWN_CLIENT) {
					connHandle->setClientType(clientTypeIt->second);
				} else {
					if (connHandle->getClientType() != clientTypeIt->second) {
						throw std::runtime_error("ConnectionHandler received a message for a clientType it did not expect.");
					}
				}

				//Now, wait on it.
				insertClientWaitingList(candidate.client_type, candidate, connHandle);
			}
		}
	}

	//New messages to process
	receiveQueue.push(MessageElement(cnnHandler, conglom));
}



void sim_mob::Broker::onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
{
	switch (eventId) {
		case sim_mob::event::EVT_CORE_AGENT_DIED: {
			const event::AgentLifeCycleEventArgs& args_ = MSG_CAST(event::AgentLifeCycleEventArgs, args);
			unRegisterEntity(args_.GetAgent());
			if (EnableDebugOutput) {
				Print() << "unregistering entity[" << args_.GetAgentId() << "]" << std::endl;
			}
			break;
		}
	}
}

AgentsList::type& sim_mob::Broker::getRegisteredAgents()
{
	//use it with caution
	return REGISTERED_AGENTS.getAgents();
}

AgentsList::type& sim_mob::Broker::getRegisteredAgents(AgentsList::Mutex* mutex)
{
	//always supply the mutex along
	mutex = REGISTERED_AGENTS.getMutex();
	return REGISTERED_AGENTS.getAgents();
}

size_t sim_mob::Broker::getClientWaitingListSize() const
{
	return clientRegistrationWaitingList.size();
}

const ClientList::Type& sim_mob::Broker::getClientList()
{
	return clientList;
}

//TODO: We need to avoid the clientType requirement. It is currently easy enough to generate unique IDs, and you can always
//      incorporate the clientType into the ID if you're not 100% sure (but for now we only serialize integers anyway).
bool sim_mob::Broker::getClientHandler(std::string clientId, std::string clientType, boost::shared_ptr<sim_mob::ClientHandler> &output) const
{
	std::map<std::string, comm::ClientType>::iterator clientTypeIt = sim_mob::Services::ClientTypeMap.find(clientType);
	if (clientTypeIt != sim_mob::Services::ClientTypeMap.end()) {
		ClientList::Type::const_iterator innerIt = clientList.find(clientTypeIt->second);
		if (innerIt != clientList.end()) {
			ClientList::Value::const_iterator finalIt = innerIt->second.find(clientId);
			if (finalIt != innerIt->second.end()) {
				output = finalIt->second;
				return true;
			}
		}
	}

	Warn() <<"Client " << clientId << " of type " << clientType << " not found" << std::endl;
	return false;
}

void sim_mob::Broker::insertClientList(std::string clientID, comm::ClientType clientType, boost::shared_ptr<sim_mob::ClientHandler> &clientHandler)
{
	{
	boost::unique_lock<boost::mutex> lock(mutex_clientList);
	clientList[clientType][clientID] = clientHandler;
	}

	if (EnableDebugOutput) {
		Print() << "connection [" <<&(*clientHandler->connHandle) << "] +1 client\n";
	}

	//+1 client for this connection.
	boost::unique_lock<boost::mutex> lock(mutex_client_done_chk);
	clientDoneChecklist[clientHandler->connHandle].total++;
	numAgents++;
}

void sim_mob::Broker::insertIntoWaitingOnWHOAMI(const std::string& token, boost::shared_ptr<sim_mob::ConnectionHandler> newConn)
{
	boost::unique_lock<boost::mutex> lock(mutex_WaitingWHOAMI);
	tokenConnectionLookup[token] = newConn;
	//waitingWHOAMI_List.push_back(newConn);
}


void  sim_mob::Broker::insertClientWaitingList(std::string clientType, ClientRegistrationRequest request, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn)
{
	boost::unique_lock<boost::mutex> lock(mutex_client_request);
	clientRegistrationWaitingList.insert(std::make_pair(clientType,ClientWaiting(request,existingConn)));
	COND_VAR_CLIENT_REQUEST.notify_one();
}
/** modified code
 * \code
//PublisherList::Value sim_mob::Broker::getPublisher(sim_mob::Services::SIM_MOB_SERVICE serviceType)
//{
//	PublisherList::Type::const_iterator it = publishers.find(serviceType);
//	if (it != publishers.end()) {
//		return it->second;
//	}
//
//	throw std::runtime_error("Publishers does not contain the specified service type.");
//}
 * \endcode
 */

sim_mob::event::EventPublisher & sim_mob::Broker::getPublisher()
{
	return publisher;
}


void sim_mob::Broker::processClientRegistrationRequests()
{
	boost::shared_ptr<ClientRegistrationHandler > handler;
	ClientWaitList::iterator it_erase;//helps avoid multimap iterator invalidation
	for (ClientWaitList::iterator it = clientRegistrationWaitingList.begin(); it != clientRegistrationWaitingList.end();) {
		//what is the client type
		std::map<std::string, comm::ClientType>::iterator clientTypeIt = sim_mob::Services::ClientTypeMap.find(it->first);
		if (clientTypeIt == sim_mob::Services::ClientTypeMap.end()) {
			std::ostringstream out("");
			out << "Undefined client type: " << it->first <<  std::endl;
			throw std::runtime_error(out.str());
		}
		comm::ClientType clientType = clientTypeIt->second;
		//find the handler for this client type.
		handler = ClientRegistrationHandlerMap[clientType];
		if (!handler) {
			std::ostringstream out("");
			out << "No Handler for [" << it->first << "] type of client" << std::endl;
			throw std::runtime_error(out.str());
		}


		if(handler->handle(*this,it->second.request, it->second.existingConn)) {
			//success: handle() just added to the client to the main client list and started its connectionHandler
			//	next, see if the waiting state of waiting-for-client-connection changes after this process
			bool wait = clientBlockers[clientType]->calculateWaitStatus();
			it_erase = it;	//keep the erase candidate. dont loose it :)

			if (EnableDebugOutput) {
				Print() << "delete from clientRegistrationWaitingList[" << it->first << "]" << std::endl;
			}
			clientRegistrationWaitingList.erase(it++);
		} else {
			it++;
		}
	}
}

void sim_mob::Broker::registerEntity(sim_mob::AgentCommUtilityBase* value)
{
	REGISTERED_AGENTS.insert(value->getEntity(), value);
	if (EnableDebugOutput) {
		Print() << std::dec;
		Print() << REGISTERED_AGENTS.size() << ":  Broker[" << this
			<< "] :  Broker::registerEntity [" << value->getEntity()->getId()
			<< "]" << std::endl;
	}

	//feedback
	value->registrationCallBack(commElement, true);

	//tell me if you are dying
	sim_mob::messaging::MessageBus::SubscribeEvent(sim_mob::event::EVT_CORE_AGENT_DIED,value->getEntity(), this);
	 COND_VAR_CLIENT_REQUEST.notify_all();
}

void sim_mob::Broker::unRegisterEntity(sim_mob::AgentCommUtilityBase *value)
{
	unRegisterEntity(value->getEntity());
}

void sim_mob::Broker::unRegisterEntity(sim_mob::Agent* agent)
{
	if (EnableDebugOutput) {
		Print() << "inside Broker::unRegisterEntity for agent[" << agent << "]\n";
	}
	//search agent's list looking for this agent
	REGISTERED_AGENTS.erase(agent);

	//search the internal container also
	duplicateEntityDoneChecker.erase(agent);

	{
	boost::unique_lock<boost::mutex> lock(mutex_clientList);
	//search registered clients list looking for this agent. whoever has it, dump him
	for(ClientList::Type::iterator it_clientType = clientList.begin(); it_clientType != clientList.end(); it_clientType++) {
		boost::unordered_map<std::string, boost::shared_ptr<sim_mob::ClientHandler> >::iterator
			it_clientID(it_clientType->second.begin()),
			it_clientID_end(it_clientType->second.end()),
			it_erase;

		for(; it_clientID != it_clientID_end; )
		{
			if(it_clientID->second->agent == agent)
			{
				it_erase = it_clientID++;
				//unsubscribe from all publishers he is subscribed to
				sim_mob::ClientHandler * clientHandler = it_erase->second.get();

				//TODO: This seems wrong; we are unsubscribing multiple times.
				for (std::set<sim_mob::Services::SIM_MOB_SERVICE>::const_iterator it=clientHandler->getRequiredServices().begin(); it!=clientHandler->getRequiredServices().end(); it++)
				{
					publisher.unSubscribeAll(clientHandler);

//					switch(*it)
//					{
//					case sim_mob::Services::SIMMOB_SRV_TIME:
//						publishers[sim_mob::Services::SIMMOB_SRV_TIME]->unSubscribe(COMMEID_TIME,clientHandler);
//						break;
//					case sim_mob::Services::SIMMOB_SRV_LOCATION:
//						publishers[sim_mob::Services::SIMMOB_SRV_LOCATION]->unSubscribe(COMMEID_LOCATION, const_cast<Agent*>(clientHandler->agent),clientHandler);
//						break;
//					case sim_mob::Services::SIMMOB_SRV_ALL_LOCATIONS:
//						publishers[sim_mob::Services::SIMMOB_SRV_ALL_LOCATIONS]->unSubscribe(COMMEID_LOCATION,(void*)COMMCID_ALL_LOCATIONS,clientHandler);
//						break;
//					}
				}
				//invalidate it and clean it up when necessary
				//don't erase it here. it may already have something to send
				//invalidation 1:
				it_erase->second->agent = nullptr;
				it_erase->second->setValidation(false);

				//Update the connection count too.
				boost::unique_lock<boost::mutex> lock(mutex_client_done_chk);
				std::map<boost::shared_ptr<sim_mob::ConnectionHandler>, ConnClientStatus>::iterator chkIt = clientDoneChecklist.find(it_erase->second->connHandle);
				if (chkIt==clientDoneChecklist.end()) {
					throw std::runtime_error("Client somehow registered without a valid connection handler.");
				}
				chkIt->second.total--;
				numAgents--;
				if (chkIt->second.total==0) {
					it_erase->second->connHandle->setValidation(false); //this is even more important
				}
			}
			else
			{
				it_clientID++;
			}
		}//inner loop

	} //outer loop
	}
}

void sim_mob::Broker::processIncomingData(timeslice now) {
	//just pop off the message queue and click handle ;)
	MessageElement msgTuple;
	while (receiveQueue.pop(msgTuple)) {
		//Conglomerates contain whole swaths of messages themselves.
		for (int i=0; i<msgTuple.conglom.getCount(); i++) {
			if (NEW_BUNDLES) {
				throw std::runtime_error("processIncoming() for NEW_BUNDLES not yet supported.");
			} else {
				const Json::Value& jsMsg = msgTuple.conglom.getMessage(i);
				if (!jsMsg.isMember("MESSAGE_TYPE")) {
					std::cout <<"Invalid message, no message_type\n";
					return;
				}

				//Certain message types have already been handled.
				std::string msgType = jsMsg["MESSAGE_TYPE"].asString();
				if (msgType=="CLIENT_MESSAGES_DONE" || msgType=="WHOAMI") {
					continue;
				}

				//Get the handler, let it parse its own expected message type.
				const sim_mob::Handler* handler = handleLookup.getHandler(msgType);
				if (handler) {
					handler->handle(msgTuple.cnnHandler, msgTuple.conglom, i, this);
				} else {
					std::cout <<"no handler for type \"" <<msgType << "\"\n";
				}
			}
		}
	}
}

bool sim_mob::Broker::frame_init(timeslice now) {
	return true;
}

Entity::UpdateStatus sim_mob::Broker::frame_tick(timeslice now) {
	return Entity::UpdateStatus::Continue;
}

//todo consider scrabbing DriverComm
bool sim_mob::Broker::allAgentUpdatesDone()
{
	return !REGISTERED_AGENTS.hasNotDone();
}



void sim_mob::Broker::agentUpdated(const Agent* target ){
	boost::unique_lock<boost::mutex> lock(mutex_agentDone);
	if(REGISTERED_AGENTS.setDone(target,true)) {
		COND_VAR_AGENT_DONE.notify_all();
	}
}

void sim_mob::Broker::onAgentUpdate(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const UpdateEventArgs& argums)
{
	const Agent* target = dynamic_cast<const Agent*>(argums.GetEntity());
	agentUpdated(target);
}


void sim_mob::Broker::onClientRegister(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const ClientRegistrationEventArgs& argums)
{
	comm::ClientType type = argums.getClientType();
	boost::shared_ptr<ClientHandler>clientHandler = argums.getClient();

	switch (type) {
	case comm::ANDROID_EMULATOR: {
		//if we are operating on android-ns3 set up,
		//each android client registration should be brought to
		//ns3's attention
		if (ConfigManager::GetInstance().FullConfig().getCommSimMode(commElement) != "android-ns3") {
			break;
		}

		//note: based on the current implementation of
		// the ns3 client registration handler, informing the
		//statistics of android clients IS a part of ns3
		//registration and configuration process. So the following implementation
		//should be executed for those android clients who will join AFTER
		//ns3's registration. So we check if the ns3 is already registered or not:
		ClientList::Type::iterator it;
		if ((it = clientList.find(comm::NS3_SIMULATOR)) == clientList.end()) {
			std::cout <<"ERROR: Android client registered without a known ns-3 handler (and one was expected).\n";
			break;
		}

		//Create the AgentsInfo message.
		std::vector<unsigned int> agentIds;
		agentIds.push_back(clientHandler->agent->getId());
		std::string message = CommsimSerializer::makeAgentsInfo(agentIds, std::vector<unsigned int>());


		//Add it.
		//TODO: ns-3 currently uses client ID 0, but it shouldn't.
		boost::shared_ptr<ClientHandler>& NS3clientHandler = it->second["0"];
		insertSendBuffer(NS3clientHandler, message);

		/*msg_header mHeader_("0", "SIMMOBILITY", "AGENTS_INFO", "SYS");
		sim_mob::comm::MsgData jMsg = JsonParser::createMessageHeader(mHeader_);
		const Agent *agent = clientHandler->agent;
		Json::Value jAgent;
		jAgent["AGENT_ID"] = agent->getId();
		//sorry it should be in an array to be compatible with the other
		//place where many agents are informed to be added
		//sorry again, compatibility issue. I will change this later.
		jMsg["ADD"].append(jAgent);

		//send to ns3's client handler
		boost::shared_ptr<ClientHandler> & NS3clientHandler = it->second["0"];
		insertSendBuffer(NS3clientHandler, jMsg);*/

		break;
	}
	default: break;

	}
}

//todo: again this function is also intrusive to Broker. find a way to move the following switch cases outside the broker-vahid
//todo suggestion: for publishment, don't iterate through the list of clients, rather, iterate the publishers list, access their subscriber list and say publish and publish for their subscribers(keep the clientlist for MHing only)
void sim_mob::Broker::processPublishers(timeslice now)
{
	for (std::vector<Services::SIM_MOB_SERVICE>::const_iterator servIt=serviceList.begin(); servIt!=serviceList.end(); servIt++) {
		switch (*servIt) {
		case sim_mob::Services::SIMMOB_SRV_TIME: {
			publisher.publish(COMMEID_TIME, TimeEventArgs(now));
			break;
		}
		case sim_mob::Services::SIMMOB_SRV_LOCATION: {
			//get to each client handler, look at his requred service and then publish for him
			for (ClientList::Type::const_iterator ctypeIt=clientList.begin(); ctypeIt!=clientList.end(); ctypeIt++) {
				for (ClientList::Value::const_iterator cidIt=ctypeIt->second.begin(); cidIt!=ctypeIt->second.end(); cidIt++) {
					const boost::shared_ptr<sim_mob::ClientHandler>& cHandler = cidIt->second;
					if(cHandler && cHandler->agent && cHandler->isValid()){//todo refine subscription list to get rid of hustle and risks
						publisher.publish(COMMEID_LOCATION, const_cast<Agent*>(cHandler->agent),LocationEventArgs(cHandler->agent));
					}
				}
			}
			break;
		}
		case sim_mob::Services::SIMMOB_SRV_ALL_LOCATIONS: {
			publisher.publish(COMMEID_ALL_LOCATIONS,(void*) COMMCID_ALL_LOCATIONS,AllLocationsEventArgs(REGISTERED_AGENTS));
			break;
		}
		case sim_mob::Services::SIMMOB_SRV_REGIONS_AND_PATH: {
			//Scan every communicating Agent and see if they need a Region or Path update sent to the client.
			//If so, publish it.
			//NOTE: This is somewhat inefficient; we can probably offload some of this responsibility to the Workers or
			//      to the EventManager itself. For now, though, its performance hit is not noticeable. ~Seth
			for (ClientList::Type::iterator listIt=clientList.begin(); listIt!=clientList.end(); listIt++) {
				for (ClientList::Value::iterator clientIt=listIt->second.begin(); clientIt!=listIt->second.end(); clientIt++) {
					if(!(clientIt->second->isValid()&&clientIt->second->agent)){
						continue;
					}
					const sim_mob::Agent* agent = clientIt->second->agent;
					if (agent->getRegionSupportStruct().isEnabled()) {
						//NOTE: Const-cast is unfortunately necessary. We could also make the Region tracking data mutable.
						//      We can't push a message back to the Broker, since it has to arrive in the same time tick
						//      (the Broker uses a half-time-tick mechanism).
						std::vector<sim_mob::RoadRunnerRegion> all_regions = const_cast<Agent*>(agent)->getAndClearNewAllRegionsSet();
						std::vector<sim_mob::RoadRunnerRegion> reg_path = const_cast<Agent*>(agent)->getAndClearNewRegionPath();
						if (!(all_regions.empty() && reg_path.empty())) {
							publisher.publish(COMMEID_REGIONS_AND_PATH, const_cast<Agent*>(agent), RegionsAndPathEventArgs(agent, all_regions, reg_path));
						}
					}
				}
			}
			break;
		}
		default:
			Warn() <<"Broker::processPubliBshers() - Unhandled service type: " <<*servIt <<"\n";
			break;
		}
	}
}

sim_mob::ClientRegistrationPublisher & sim_mob::Broker::getRegistrationPublisher()
{
	return registrationPublisher;
}

void sim_mob::Broker::sendReadyToReceive()
{
	//Iterate over all clients actually waiting in the client list.
	//NOTE: This is slightly different than how the previous code did it, but it *should* work.
	//It will at least fail predictably: if the simulator freezes in the first time tick for a new agent, this is where to look.
	//TODO: We need a better way of tracking <client,destAgentID> pairs anyway; that fix will likely simplify this function.
	std::map<SendBuffer::Key, std::string> pendingMessages;
	for (std::map<SendBuffer::Key, OngoingSerialization>::const_iterator it=sendBuffer.begin(); it!=sendBuffer.end(); it++) {
		pendingMessages[it->first] = CommsimSerializer::makeReadyToReceive();
	}

	for (std::map<SendBuffer::Key, std::string>::const_iterator it=pendingMessages.begin(); it!=pendingMessages.end(); it++) {
		insertSendBuffer(it->first, it->second);
	}



	//ClientList::Pair clientByType;
	//ClientList::ValuePair clientByID;
	//boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
	//msg_header msg_header_;
/*	{
		boost::unique_lock<boost::mutex> lock(mutex_clientList);
		for (ClientList::Type::iterator listIt=clientList.begin(); listIt!=clientList.end(); listIt++) {
			for (ClientList::Value::iterator clientIt=listIt->second.begin(); clientIt!=listIt->second.end(); clientIt++) {

//		BOOST_FOREACH(clientByType, clientList) {
			//BOOST_FOREACH(clientByID, clientByType.second) {
				const boost::shared_ptr<sim_mob::ClientHandler>& clnHandler = clientIt->second;
				msg_header msgHeader;
				msgHeader.msg_cat = "SYS";
				msgHeader.msg_type = "READY_TO_RECEIVE";
				msgHeader.sender_id = "0";
				msgHeader.sender_type = "SIMMOBILITY";
				sim_mob::comm::MsgData msg = JsonParser::createMessageHeader(msgHeader);
				insertSendBuffer(clnHandler,  msg);
			}
		}
	}*/
}

void sim_mob::Broker::processOutgoingData(timeslice now)
{
	for (std::map<SendBuffer::Key, OngoingSerialization>::iterator it=sendBuffer.begin(); it!=sendBuffer.end(); it++) {
		boost::shared_ptr<sim_mob::ConnectionHandler> conn = it->first->connHandle;

		//Our data stream contains several messages pointing to different clients. We need to
		// de-multiplex these, creating a mega-"message" of type Json::Value for each client.
		//std::map<boost::shared_ptr<sim_mob::ClientHandler>, Json::Value> messages;

		/*SendBufferItem datum;
		while (data.pop(datum)) {
			//TODO: This seems un-needed; can't we just .append() it? ~Seth
			if (messages.find(datum.client)==messages.end()) {
				messages[datum.client].clear();
			}
			messages[datum.client].append(datum.msg);
		}*/

		//Getting the string is easy:
		BundleHeader header;
		std::string message;
		if (!CommsimSerializer::serialize_end(it->second, header, message)) {
			throw std::runtime_error("Broker: Could not finalize serialization.");
		}

		//Forward to the given client.
		//TODO: We can add per-client routing here.
		conn->forwardMessage(message);

		//build a jsoncpp structure (per client) comprising of a header and data array(containing messages)
		/*for (std::map<boost::shared_ptr<sim_mob::ClientHandler>, Json::Value>::const_iterator msgIt=messages.begin(); msgIt!=messages.end(); msgIt++) {
			//TODO: Is the .clear() really needed? ~Seth
			Json::Value jpacket;
			jpacket.clear();

			//Make sure we have something to send.
			int numMsgs = msgIt->second.size();
			if (numMsgs == 0) {
				continue;
			}

			//Write the header; this will route the message properly once it reaches the TCP relay.
			Json::Value jheader = JsonParser::createPacketHeader(pckt_header(numMsgs, msgIt->first->clientId));
			jpacket["PACKET_HEADER"] = jheader;

			//Write the data, serialize it.
			jpacket["DATA"] = msgIt->second;
			std::string str = Json::FastWriter().write(jpacket);

			//Forward to the given client.
			//TODO: We can add per-client routing here.
			cnn->forwardMessage(str);
		}*/
	}

	//Clear the buffer for the next time tick.
	sendBuffer.clear();
}

//checks to see if the subscribed entity(agent) is alive
bool sim_mob::Broker::deadEntityCheck(sim_mob::AgentCommUtilityBase * info)
{
	if (!info) {
		throw std::runtime_error("Invalid AgentCommUtility\n");
	}

	Agent * target = info->getEntity();
	try {

		if (!(target->currWorkerProvider)) {
//			Print() << "1-deadEntityCheck for[" << target << "]" << std::endl;
			return true;
		}

		//one more check to see if the entity is deleted
		const std::set<sim_mob::Entity*> & managedEntities_ =
				target->currWorkerProvider->getEntities();
		std::set<sim_mob::Entity*>::const_iterator it =
				managedEntities_.begin();
		if (!managedEntities_.size()) {
//			Print() << "2-deadEntityCheck for[" << target << "]" << std::endl;
			return true;
		}
		for (std::set<sim_mob::Entity*>::const_iterator it =
				managedEntities_.begin(); it != managedEntities_.end(); it++) {
			//agent is still being managed, so it is not dead
			if (*it == target)
				return false;
		}
	} catch (std::exception& e) {
//		Print() << "3-deadEntityCheck for[" << target << "]" << std::endl;
		return true;
	}

//	Print() << "4-deadEntityCheck for[" << target << "]" << std::endl;

	return true;
}

//iterate the entire agent registration list looking for
//those who are not done with their update and check if they are dead.
//you better hope they are dead otherwise you have to hold the simulation
//tick waiting for them to finish
void sim_mob::Broker::refineSubscriptionList()
{
	if (EnableDebugOutput) {
		Print() << "inside Broker::refineSubscriptionList" << std::endl;
	}

	//do all the operation using the objects's mutex
	boost::function<void(sim_mob::Agent*)> Fn = boost::bind(
			&Broker::refineSubscriptionList, this, _1);
	REGISTERED_AGENTS.for_each_agent(Fn);
}

//tick waiting for them to finish
void sim_mob::Broker::refineSubscriptionList(sim_mob::Agent * target)
{
	//you or your worker are probably dead already. you just don't know it
	if (!target->currWorkerProvider) {
		if (EnableDebugOutput) {
			Print() << "1-refine subscription for agent [" << target << "]" << std::endl;
		}
		unRegisterEntity(target);
		return;
	}
	const std::set<sim_mob::Entity*> & managedEntities_ =
			(target->currWorkerProvider)->getEntities();
	std::set<sim_mob::Entity*>::const_iterator it_entity = std::find(
			managedEntities_.begin(), managedEntities_.end(), target);
	if (it_entity == managedEntities_.end()) {
		if (EnableDebugOutput) {
			Print() << "2-refine subscription for agent [" << target << "]" << std::endl;
		}
		unRegisterEntity(target);
		return;
	}
}
//sim_mob::Broker sim_mob::Broker::instance(MtxStrat_Locked, 0);

//todo:  put a better condition here. this is just a placeholder
bool sim_mob::Broker::isWaitingForAgentRegistration() const
{
	sim_mob::BrokerBlockers::Value blocker = agentBlockers.at(0);
	bool res = blocker->calculateWaitStatus();
	return res;
}

//todo:  put a better condition here. this is just a placeholder
bool sim_mob::Broker::clientsQualify() const
{
	return clientList.size() >= MIN_CLIENTS;
}

size_t sim_mob::Broker::getNumConnectedAgents() const
{
	return numAgents;
}

//returns true if you need to wait
bool sim_mob::Broker::isWaitingForAnyClientConnection()
{
//	Print() << "inside isWaitingForAnyClientConnection " << clientBlockers.size() << std::endl;
	//BrokerBlockers::Pair pp;
	int i = 0;
	for (BrokerBlockers::Type::const_iterator it=clientBlockers.begin(); it!=clientBlockers.end(); it++) {
	//BOOST_FOREACH(pp, clientBlockers) {
		if (it->second->isWaiting()) {
			if (EnableDebugOutput) {
				Print() << " isWaitingForAnyClientConnection[" << i << "] : wait" << std::endl;
			}
			return true;
		}
		i++;
	}
//	Print() << "isWaitingForAnyClientConnection : Dont wait" << std::endl;
	return false;
}

bool sim_mob::Broker::waitAndAcceptConnections() {
	//	Initial evaluation
	{
		boost::unique_lock<boost::mutex> lock(mutex_client_request);
		processClientRegistrationRequests();
		bool res1 = isWaitingForAgentRegistration();
		bool res2 = isWaitingForAnyClientConnection();
		bool res3 = !res1 && !res2;
		bool res = /*brokerCanTickForward || */res3;
		brokerCanTickForward = res;
	}

	/**if:
	 * 1- number of subscribers is too low
	 * 2-there is no client(emulator) waiting in the queue
	 * 3-this update function never started to process any data so far
	 * then:
	 *  wait for enough number of clients and agents to join
	 */

	{
		boost::unique_lock<boost::mutex> lock(mutex_client_request);
		while (!brokerCanTickForward) {
			if (EnableDebugOutput) {
				Print() << " brokerCanTickForward->WAITING" << std::endl;
			}
			COND_VAR_CLIENT_REQUEST.wait(lock);
			if (EnableDebugOutput) {
				Print() << "COND_VAR_CLIENT_REQUEST released" << std::endl;
			}
			processClientRegistrationRequests();

			bool res1 = isWaitingForAgentRegistration();
			bool res2 = isWaitingForAnyClientConnection();
			bool res3 = !res1 && !res2;
			bool res = /*brokerCanTickForward ||*/res3;
			brokerCanTickForward = res;
			if (EnableDebugOutput) {
				Print() << "brokerCanTickForward[" << res1 << "-" << res2 << "]" << std::endl;
			}
			//	brokerCanTickForward = brokerCanTickForward || ((isWaitingForAgentRegistration() && !isWaitingForAnyClientConnection()));
//		Print() << "Broker::wait()::Secondary Evaluation => " << (brokerCanTickForward ? "True" : "false") << std::endl;
		}
	}

	//broker started before but suddenly is no more qualified to run
//	if(brokerCanTickForward && (!isWaitingForAgentRegistration())) {
//		//don't block, just cooperate & don't do anything until this simulation ends
//		//TODO: This might be why our client eventually gives up.
//		return false;
//	}

	//Success! Continue.
	return true;
}

void sim_mob::Broker::waitForAgentsUpdates()
{
	boost::unique_lock<boost::mutex> lock(mutex_agentDone);
	while(!allAgentUpdatesDone()) {
		if (EnableDebugOutput) {
			Print() << "waitForAgentsUpdates _WAIT" << std::endl;
		}
		COND_VAR_AGENT_DONE.wait(lock);
		if (EnableDebugOutput) {
			Print() << "waitForAgentsUpdates _WAIT_released" << std::endl;
		}
	}
}


bool sim_mob::Broker::allClientsAreDone()
{
	//ClientList::Pair clientByType;
	//ClientList::ValuePair clientByID;

	boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
	msg_header msg_header_;
	boost::unique_lock<boost::mutex> lock(mutex_clientList);

	for (ClientList::Type::const_iterator ctypeIt=clientList.begin(); ctypeIt!=clientList.end(); ctypeIt++) {
		for (ClientList::Value::const_iterator cidIt=ctypeIt->second.begin(); cidIt!=ctypeIt->second.end(); cidIt++) {
	//BOOST_FOREACH(clientByType, clientList) {
		//BOOST_FOREACH(clientByID, clientByType.second) {
			clnHandler = cidIt->second;
			//If we have a valid client handler.
			if (clnHandler && clnHandler->isValid()) {
				//...and a valid connection handler.
				if (clnHandler->connHandle && clnHandler->connHandle->isValid() && clnHandler->connHandle->is_open()) {
					//Check if this connection's "done" count equals its "total" known agent count.
					//TODO: This actually checks the same connection handler multiple times if connections are multiplexed
					//      (one for each ClientHandler). This is harmless, but inefficient.
					boost::unique_lock<boost::mutex> lock(mutex_client_done_chk);
					std::map<boost::shared_ptr<sim_mob::ConnectionHandler>, ConnClientStatus>::iterator chkIt = clientDoneChecklist.find(clnHandler->connHandle);
					if (chkIt==clientDoneChecklist.end()) {
						throw std::runtime_error("Client somehow registered without a valid connection handler.");
					}
					if (chkIt->second.done < chkIt->second.total) {
						if (EnableDebugOutput) {
							Print() << "connection [" <<&(*clnHandler->connHandle) << "] not done yet: " <<chkIt->second.done <<" of " <<chkIt->second.total <<"\n";
						}
						return false;
					}
				}
			}
		}
	}
	return true;
}

Entity::UpdateStatus sim_mob::Broker::update(timeslice now)
{
	if (EnableDebugOutput) {
		Print() << "Broker tick:" << now.frame() << std::endl;
	}

	//step-1 : Create/start the thread if this is the first frame.
	//TODO: transfer this to frame_init
	if (now.frame() == 0) {
		connection->start();
	}

	if (EnableDebugOutput) {
		Print() << "=====================ConnectionStarted =======================================\n";
	}

	//Step-2: Ensure that we have enough clients to process
	//(in terms of client type (like ns3, android emulator, etc) and quantity(like enough number of android clients) ).
	//Block the simulation here(if you have to)
	waitAndAcceptConnections();

	if (EnableDebugOutput) {
		Print() << "===================== wait Done =======================================\n";
	}

	//step-3: Process what has been received in your receive container(message queue perhaps)
	processIncomingData(now);

	if (EnableDebugOutput) {
		Print() << "===================== processIncomingData Done =======================================\n";
	}

	//step-4: if need be, wait for all agents(or others)
	//to complete their tick so that you are the last one ticking)
	waitForAgentsUpdates();

	if (EnableDebugOutput) {
		Print()<< "===================== waitForAgentsUpdates Done =======================================\n";
	}

	//step-5: signal the publishers to publish their data
	processPublishers(now);

	if (EnableDebugOutput) {
		Print() << "===================== processPublishers Done =======================================\n";
	}

//	step-5.5:for each client, append a message at the end of all messages saying Broker is ready to receive your messages
	sendReadyToReceive();

	//step-6: Now send all what has been prepared, by different sources, to their corresponding destications(clients)
	processOutgoingData(now);

	if (EnableDebugOutput) {
		Print() << "===================== processOutgoingData Done =======================================\n";
	}

	//step-7:
	//the clients will now send whatever they want to send(into the incoming messagequeue)
	//followed by a Done! message.That is when Broker can go forwardClientList::pair clientByType;
	waitForClientsDone();

	if (EnableDebugOutput) {
		Print() << "===================== waitForClientsDone Done =======================================\n";
	}

	//step-9: final steps that should be taken before leaving the tick
	//prepare for next tick.
	cleanup();
	return UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void sim_mob::Broker::removeClient(ClientList::Type::iterator it_erase)
{
	//todo: enable this later
//	Print() << "Broker::removeClient locking mutex_clientList" << std::endl;
//	if(!it_erase->second)
//	{
//		return;
//	}
//	//delete the connection handler inside the client handler
//	if(it_erase->second->cnnHandler)
//	{
////		it_erase->second->cnnHandler.reset();
//	}
//	//delete the clientHandler
////	it_erase->second.reset();
//	//remove it from the list
//	clientList.erase(it_erase);
//	Print() << "Broker::removeClient UNlocking mutex_clientList" << std::endl;
}

void sim_mob::Broker::waitForClientsDone()
{
	boost::unique_lock<boost::mutex> lock(mutex_clientDone);
	while (!allClientsAreDone()) {
		COND_VAR_CLIENT_DONE.wait(lock);
	}
}

void sim_mob::Broker::cleanup()
{
	//for internal use
	duplicateEntityDoneChecker.clear();

	//clientDoneChecker.clear();
	boost::unique_lock<boost::mutex> lock(mutex_client_done_chk);
	std::map<boost::shared_ptr<sim_mob::ConnectionHandler>, ConnClientStatus>::iterator chkIt = clientDoneChecklist.begin();
	for (;chkIt!=clientDoneChecklist.end(); chkIt++) {
		chkIt->second.done = 0;
	}

	return;

	//note:this part is supposed to delete clientList entries for the dead agents
	//But there is a complication on deleting connection handler
	//there is a chance the sockets are deleted before a send ACK arrives

}

std::map<std::string, sim_mob::Broker*>& sim_mob::Broker::getExternalCommunicators()
{
	return externalCommunicators;
}

sim_mob::Broker* sim_mob::Broker::getExternalCommunicator(const std::string & value)
{
	if (externalCommunicators.find(value) != externalCommunicators.end()) {
		return externalCommunicators[value];
	}
	return 0;
}

void sim_mob::Broker::addExternalCommunicator(const std::string & name, sim_mob::Broker* broker)
{
	externalCommunicators.insert(std::make_pair(name, broker));
}

//abstracts & virtuals
void sim_mob::Broker::load(const std::map<std::string, std::string>& configProps)
{
}

void sim_mob::Broker::frame_output(timeslice now)
{
}

bool sim_mob::Broker::isNonspatial()
{
	return true;
}



