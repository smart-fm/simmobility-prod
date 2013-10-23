//external libraries
#include "Broker.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <json/json.h>
#include <sstream>
//core simmobility
#include "entities/AuraManager.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "workers/WorkGroup.hpp"
#include "workers/Worker.hpp"
//communication simulator
#include "Common.hpp"
#include "entities/commsim/message/derived/roadrunner-android/RR_Factory.hpp"//todo :temprorary
#include "entities/commsim/message/derived/roadrunner-android-ns3/roadrunner_android_factory.hpp"//todo :temprorary
#include "entities/commsim/comm_support/AgentCommUtility.hpp"
#include "entities/commsim/connection/ConnectionServer.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
//temporary, used for hardcoding publishers in the constructor
//#include "entities/commsim/service/derived/LocationPublisher.hpp"
//#include "entities/commsim/service/derived/TimePublisher.hpp"
#include "entities/commsim/wait/WaitForAndroidConnection.hpp"
#include "entities/commsim/wait/WaitForNS3Connection.hpp"
#include "entities/commsim/wait/WaitForAgentRegistration.hpp"
#include "event/SystemEvents.hpp"
#include "message/MessageBus.hpp"

#include "event/EventPublisher.hpp"

namespace{
//subclassed Eventpublisher coz its destructor is pure virtual
	class BrokerPublisher: public sim_mob::event::EventPublisher  {
	public:
		BrokerPublisher(){}
		virtual ~BrokerPublisher(){}
	};
}


namespace sim_mob
{
std::map<std::string, sim_mob::Broker*> Broker::externalCommunicators;
void Broker::enable() { enabled = true; }
void Broker::disable() { enabled = false; }
bool Broker::isEnabled() const { return enabled; }

bool Broker::insertSendBuffer(boost::shared_ptr<sim_mob::ConnectionHandler> cnnHandler, Json::Value &value )
{
	if(!cnnHandler) {
		return false;
	}
	if (cnnHandler->isValid() == false) {
		return false;
	}
	sendBuffer[cnnHandler].add(value);
}
Broker::Broker(const MutexStrategy& mtxStrat, int id )
: Agent(mtxStrat, id)
,enabled(true), configured(false)
{
	//Various Initializations
	connection.reset(new ConnectionServer(*this));
	 brokerCanTickForward = false;
	 m_messageReceiveCallback = boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)>
	 	(boost::bind(&Broker::messageReceiveCallback,this, _1, _2));


	 Print() << "Creating Broker [" << this << "]" << std::endl;

		if(!configured)
		{
			configure();
		}


}

void Broker::configure() {
	sim_mob::Worker::GetUpdatePublisher().Subscribe(sim_mob::event::EVT_CORE_AGENT_UPDATED, (void*)sim_mob::event::CXT_CORE_AGENT_UPDATE, this, CONTEXT_CALLBACK_HANDLER(UpdateEventArgs, Broker::onAgentUpdate));
//	Print() << "Broker sunscribed to agent done" << std::endl;
	 //todo, for the following maps , think of something non intrusive to broker. This is merely hardcoding-vahid
	 if(ConfigManager::GetInstance().FullConfig().getAndroidClientType() == "android-ns3")
	 {
		BrokerPublisher* onlyLocationsPublisher = new BrokerPublisher();
		onlyLocationsPublisher->RegisterEvent(COMMEID_LOCATION);
		//publishers
		publishers.insert(
				std::make_pair(SIMMOB_SRV_LOCATION,
						PublisherList::dataType(onlyLocationsPublisher)));

		BrokerPublisher* allLocationsPublisher = new BrokerPublisher();
		allLocationsPublisher->RegisterEvent(COMMEID_LOCATION);
		publishers.insert(
				std::make_pair(SIMMOB_SRV_ALL_LOCATIONS,
						PublisherList::dataType(allLocationsPublisher)));

		BrokerPublisher* timePublisher = new BrokerPublisher();
		timePublisher->RegisterEvent(COMMEID_TIME);
		publishers.insert(
				std::make_pair(SIMMOB_SRV_TIME,
						PublisherList::dataType(timePublisher)));
		//listen to publishers who announce registration of new clients...
		ClientRegistrationHandler::getPublisher().Subscribe(ConfigParams::ANDROID_EMULATOR, this, CALLBACK_HANDLER(ClientRegistrationEventArgs, Broker::onClientRegister));
		ClientRegistrationHandler::getPublisher().Subscribe(ConfigParams::NS3_SIMULATOR, this, CALLBACK_HANDLER(ClientRegistrationEventArgs, Broker::onClientRegister));

		//current message factory
		//todo: choose a factory based on configurations not hardcoding
		boost::shared_ptr<
				sim_mob::MessageFactory<std::vector<msg_ptr>&, std::string&> > android_factory(
				new sim_mob::rr_android_ns3::RR_Android_Factory());
		boost::shared_ptr<
				sim_mob::MessageFactory<std::vector<msg_ptr>&, std::string&> > ns3_factory(
				new sim_mob::rr_android_ns3::RR_NS3_Factory());

		messageFactories.insert(
				std::make_pair(ConfigParams::ANDROID_EMULATOR,
						android_factory));
		messageFactories.insert(
				std::make_pair(ConfigParams::NS3_SIMULATOR, ns3_factory));

		// wait for connection criteria for this broker
		clientBlockers.insert(
				std::make_pair(ConfigParams::ANDROID_EMULATOR,
						boost::shared_ptr<WaitForAndroidConnection>(
								new WaitForAndroidConnection(*this,MIN_CLIENTS))));
		clientBlockers.insert(
				std::make_pair(ConfigParams::NS3_SIMULATOR,
						boost::shared_ptr<WaitForNS3Connection>(
								new WaitForNS3Connection(*this))));
		agentBlockers.insert(
				std::make_pair(0,
						boost::shared_ptr<WaitForAgentRegistration>(
								new WaitForAgentRegistration(*this,MIN_AGENTS))));
	 }
	 else
		 if(ConfigManager::GetInstance().FullConfig().getAndroidClientType() == "android-only") {
				//publishers

				BrokerPublisher* onlyLocationsPublisher = new BrokerPublisher();
				onlyLocationsPublisher->RegisterEvent(COMMEID_LOCATION);
				//publishers
				publishers.insert(
						std::make_pair(SIMMOB_SRV_LOCATION,
								PublisherList::dataType(onlyLocationsPublisher)));

				BrokerPublisher* timePublisher = new BrokerPublisher();
				timePublisher->RegisterEvent(COMMEID_TIME);
				publishers.insert(
						std::make_pair(SIMMOB_SRV_TIME,
								PublisherList::dataType(timePublisher)));
				//listen to publishers who announce registration of new clients...
				ClientRegistrationHandler::getPublisher().Subscribe(ConfigParams::ANDROID_EMULATOR, this, CALLBACK_HANDLER(ClientRegistrationEventArgs, Broker::onClientRegister));
				//current message factory
				//todo: choose a factory based on configurations not hardcoding
				boost::shared_ptr<
						sim_mob::MessageFactory<std::vector<msg_ptr>&, std::string&> > android_factory(
						new sim_mob::roadrunner::RR_Factory());
				//note that both client types refer to the same message factory belonging to roadrunner application. we will modify this to a more generic approach later-vahid
				messageFactories.insert(
						std::make_pair(ConfigParams::ANDROID_EMULATOR,
								android_factory));

				// wait for connection criteria for this broker
				clientBlockers.insert(
						std::make_pair(ConfigParams::ANDROID_EMULATOR,
								boost::shared_ptr<WaitForAndroidConnection>(
										new WaitForAndroidConnection(*this, MIN_CLIENTS))));
				agentBlockers.insert(
						std::make_pair(0,
								boost::shared_ptr<WaitForAgentRegistration>(
										new WaitForAgentRegistration(*this, MIN_AGENTS))));

		 }
//	 Print() << "Broker constructor()=>androidClientType[" << ConfigManager::GetInstance().FullConfig().getAndroidClientType() << "]"
//			 <<" clientBlockers.size()=" << clientBlockers.size() << std::endl;
	 configured = true;
}

Broker::~Broker()
{
}
/*
 * the callback function called by connection handlers to ask
 * the Broker what to do when a message arrives(this method is to be called
 * after whoareyou, registration and connectionHandler creation....)
 * anyway, what is does is :extract messages from the input string
 * (remember that a message object carries a reference to its handler also(except for "CLIENT_MESSAGES_DONE" messages)
 */
void Broker::messageReceiveCallback(boost::shared_ptr<ConnectionHandler> cnnHandler, std::string input)
{
	boost::shared_ptr<MessageFactory<std::vector<msg_ptr>&, std::string&> > m_f = messageFactories[cnnHandler->clientType];
	std::vector<msg_ptr> messages;
	m_f->createMessage(input, messages);
//	post the messages into the message queue one by one(add their cnnHandler also)
	for(std::vector<msg_ptr>::iterator it = messages.begin(); it != messages.end(); it++)
	{

		msg_data_t &data = it->get()->getData();
		std::string type = data["MESSAGE_TYPE"].asString();
//		Print() << "Received " << (cnnHandler->clientType == ConfigParams::NS3_SIMULATOR ? "NS3" : ( cnnHandler->clientType == ConfigParams::ANDROID_EMULATOR ? "ANDROID" : "UNKNOWN_TYPE") ) << " : " << type << std::endl;
		if(type == "CLIENT_MESSAGES_DONE")
		{
			boost::unique_lock<boost::mutex> lock(mutex_clientDone);
			//update() will wait until all clients send this message for each tick
//			Print()
//								<< "received CLIENT_MESSAGES_DONE for client["
//								<< cnnHandler->clientType << ":" << cnnHandler->clientID << "]" << std::endl;
			clientDoneChecker.insert(cnnHandler);
			COND_VAR_CLIENT_DONE.notify_one();
		}
		else
		{
			receiveQueue.post(boost::make_tuple(cnnHandler,*it));
		}
	}
}

boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> Broker::getMessageReceiveCallBack() {
	return m_messageReceiveCallback;
}

/*void Broker::OnAgentFinished(sim_mob::event::EventId eventId, EventPublisher* sender, const AgentLifeEventArgs& args){
//	Print() << "Agent " << args.GetAgent() << "  is dying" << std::endl;
	unRegisterEntity(args.GetAgent());
	//FUTURE when we have reentrant locks inside of Publisher.
	//const_cast<Agent*>(agent)->UnSubscribe(AGENT_LIFE_EVENT_FINISHED_ID, this);
}*/
void Broker::OnEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args){

	switch(eventId){
		case sim_mob::event::EVT_CORE_AGENT_DIED:{
			const event::AgentLifeCycleEventArgs& args_ = MSG_CAST(event::AgentLifeCycleEventArgs, args);
			Print() << "Broker::UNregisterINGEntity[" << args_.GetAgent() << "][" << args_.GetAgentId() << "]" << std::endl;
			unRegisterEntity(args_.GetAgent());
			break;
		}
	default:break;
	}

}
AgentsList::type &Broker::getRegisteredAgents() {
	//use it with caution
	return REGISTERED_AGENTS.getAgents();
}

AgentsList::type& Broker::getRegisteredAgents(AgentsList::Mutex* mutex) {
	//always supply the mutex along
	mutex = REGISTERED_AGENTS.getMutex();
	return REGISTERED_AGENTS.getAgents();
}

ClientWaitList & Broker::getClientWaitingList(){
	return clientRegistrationWaitingList;
}

ClientList::type & Broker::getClientList() {
	return clientList;
}


bool Broker::getClientHandler(std::string clientId,std::string clientType, boost::shared_ptr<sim_mob::ClientHandler> &output)
{

	//use try catch to use map's .at() and search only once
	try
	{
		ConfigParams::ClientType clientType_ = ClientTypeMap.at(clientType);
		boost::unordered_map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > & inner = clientList[clientType_];
		try
		{
			output = inner.at(clientId); //this is what we are looking for
			return true;
		}
		catch(std::out_of_range &e)
		{
			WarnOut("Client " <<  clientId << " of type " <<  clientType << " not found" << std::endl);
			return false;
		}

	}
	catch(std::out_of_range &e)
	{

		Print() << "Client type" <<  clientType << " not found" << std::endl;
		return false;
	}
	//program never reaches here :)
	return false;
}

void Broker::insertClientList(std::string clientID, unsigned int clientType, boost::shared_ptr<sim_mob::ClientHandler> &clientHandler){
	boost::unique_lock<boost::mutex> lock(mutex_clientList);
	clientList[clientType][clientID] = clientHandler;
}

void  Broker::insertClientWaitingList(std::pair<std::string,ClientRegistrationRequest > p)//pair<client type, request>
{
	boost::unique_lock<boost::mutex> lock(mutex_client_request);
//	Print() << "Inserting into clientRegistrationWaitingList" << std::endl;
	clientRegistrationWaitingList.insert(p);
	COND_VAR_CLIENT_REQUEST.notify_one();
}

PublisherList::type & Broker::getPublishers()
{
	return publishers;
}

void Broker::processClientRegistrationRequests()
{
	boost::shared_ptr<ClientRegistrationHandler > handler;
	ClientWaitList::iterator it_erase;//helps avoid multimap iterator invalidation
	for(ClientWaitList::iterator it = clientRegistrationWaitingList.begin(), it_end(clientRegistrationWaitingList.end()); it != it_end;/*no ++ here */  )
	{
		handler = clientRegistrationFactory.getHandler((ClientTypeMap[it->first]));
		if(handler->handle(*this,it->second))
		{
			//success: handle() just added to the client to the main client list and started its connectionHandler
			//	next, see if the waiting state of waiting-for-client-connection changes after this process
			bool wait = clientBlockers[ClientTypeMap[it->first]]->calculateWaitStatus();
//			Print() << "calculateWaitStatus for  " << it->first << "  " << (wait? "WAIT " : "DONT_WAIT") << std::endl;
			if(!wait)
			{
				//	then, get this request out of registration list.
				it_erase =  it++;//keep the erase candidate. dont loose it :)
				clientRegistrationWaitingList.erase(it_erase) ;
				//note: if needed,remember to do the necessary work in the
				//corresponding agent w.r.t the result of handle()
				//do this through a callback to agent's reuest
			}
			else
			{
				it++; //putting it here coz multimap is not like a vector. erase doesn't return an iterator.
			}
		}
		else
		{
			Print() << "clientRegistration handler for " << it->first << " failed" << std::endl;
			it++; //putting it here coz multimap is not like a vector. erase doesn't return an iterator.
		}
	}
}

bool  Broker::registerEntity(sim_mob::AgentCommUtilityBase* value)
{
	Print() << std::dec;
//	registeredAgents.insert(std::make_pair(value->getEntity(), value));
	REGISTERED_AGENTS.insert(value->getEntity(), value);
	Print()<< REGISTERED_AGENTS.size() << ":  Broker[" << this <<  "] :  Broker::registerEntity [" << value->getEntity()->getId() << "]" << std::endl;
	//feedback
	value->registrationCallBack(true);
	//tell me if you are dying
	sim_mob::messaging::MessageBus::SubscribeEvent(sim_mob::event::EVT_CORE_AGENT_DIED,
			static_cast<event::Context>(value->getEntity()), this);
	return true;
}

void  Broker::unRegisterEntity(sim_mob::AgentCommUtilityBase *value)
{
	unRegisterEntity(value->getEntity());
}

void  Broker::unRegisterEntity(sim_mob::Agent * agent)
{
	Print() << "inside Broker::unRegisterEntity for agent[" << agent << "]" << std::endl;
	//search agent's list looking for this agent
//	registeredAgents.erase(agent); //hopefully the agent is there
	REGISTERED_AGENTS.erase(agent);

	//search the internal container also
	duplicateEntityDoneChecker.erase(agent);

	{
		boost::unique_lock<boost::mutex> lock(mutex_clientList);
	//search registered clients list looking for this agent. whoever has it, dump him
	for(ClientList::iterator it_clientType = clientList.begin(); it_clientType != clientList.end(); it_clientType++)
	{
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
				sim_mob::SIM_MOB_SERVICE srv;
				BOOST_FOREACH(srv, clientHandler->requiredServices)
				{
					switch(srv)
					{
					case SIMMOB_SRV_TIME:
						publishers[SIMMOB_SRV_TIME]->UnSubscribe(COMMEID_TIME,clientHandler);
						break;
					case SIMMOB_SRV_LOCATION:
						publishers[SIMMOB_SRV_LOCATION]->UnSubscribe(COMMEID_LOCATION,(void*)clientHandler->agent,clientHandler);
						break;
					case SIMMOB_SRV_ALL_LOCATIONS:
						publishers[SIMMOB_SRV_ALL_LOCATIONS]->UnSubscribe(COMMEID_LOCATION,(void*)COMMCID_ALL_LOCATIONS,clientHandler);
						break;
					}
				}
				//erase him from the list
				//clientList.erase(it_erase);
				//don't erase it here. it may already have something to send
				//invalidate it and clean it up when necessary
				//invalidation 1:
				it_erase->second->agent = 0;
				it_erase->second->cnnHandler->agentPtr = 0; //this is even more important
				//or a better version //todo: use one version only
				it_erase->second->setValidation(false);
				it_erase->second->cnnHandler->setValidation(false); //this is even more important

			}
			else
			{
				it_clientID++;
			}
		}//inner loop

	}//outer loop
	}
}

void Broker::processIncomingData(timeslice now)
{
	//just pop off the message queue and click handl ;)
	MessageElement::type msgTuple;
	while(receiveQueue.pop(msgTuple))
	{
		msg_ptr &msg = msgTuple.get<1>();
		msg_data_t data = msg->getData();
		Json::FastWriter w;
		msg->supplyHandler()->handle(msg,this);
	}
}

bool Broker::frame_init(timeslice now){
	return true;
};
Entity::UpdateStatus Broker::frame_tick(timeslice now){
	return Entity::UpdateStatus::Continue;
}

//todo consider scrabbing DriverComm
bool Broker::allAgentUpdatesDone()
{
//now, with the help of eventpublisher we dont need all this expensive ops-vahid
//	AgentsMap::iterator it = registeredAgents.begin(), it_end(registeredAgents.end()), it_erase;
//
//	int i = 0;
//	while(it != it_end)
//	{
//		sim_mob::AgentCommUtilityBase *info = (it->second);//easy read
//			try
//			{
//				if (info->isAgentUpdateDone())
//				{
//					duplicateEntityDoneChecker.insert(it->first);
//				}
//				else
//				{
////					Print() << "Agent is registered but its update not done : " <<  it->first << std::endl;
//				}
//			}
//			catch(std::exception& e)
//			{
//				WarnOut( "Exception Occured " << e.what() << std::endl);
//				Print() << "Exception Occured " << e.what() << std::endl;
//				if(deadEntityCheck(info))
//				{
//					Print()<< "deadEntityCheck works " << info->getEntity() << std::endl;
//					it++;
//					unRegisterEntity(info);
//					continue;
//				}
//				else
//					throw std::runtime_error("Unknown Error checking entity");
//			}
//			it++;
//	}
	;
//	if(!(res= (duplicateEntityDoneChecker.size() >= registeredAgents.size())))
	AgentsList::done_range its= REGISTERED_AGENTS.getNotDone();
	bool res = its.first == its.second;
	if(!res) {
		Print() << "allAgentUpdatesDone not done : " <<  std::endl;

	for (; its.first != its.second; its.first++) {
		Print() << "Not Done: [" << its.first->agent->getId() << "]"
				<< std::endl;
		}
	}
	return res;
}

void Broker::onAgentUpdate(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const UpdateEventArgs& argums){

	Print() << "Inside onAgentUpdate" << std::endl;
	Agent * target = const_cast<Agent*>(dynamic_cast<const Agent*>(argums.GetEntity()));
	Print() << "onAgentUpdate:: setting[" << target->getId() << "] to.done" << std::endl;
	boost::unique_lock<boost::mutex> lock(mutex_agentDone);
	if(REGISTERED_AGENTS.setDone(target,true))
	{
		Print() << "Agent[" << target->getId() << "] done" << std::endl;
//		duplicateEntityDoneChecker.insert(target);
		COND_VAR_AGENT_DONE.notify_all();
	}
	else
	{
		Print() << "Thread[" << boost::this_thread::get_id() << "] Discarded Agent[" << target->getId() << "]" << std::endl;
	}
}

void Broker::AgentUpdated(Agent * target){

	Print() << "Inside AgentUpdated" << std::endl;
	Print() << "AgentUpdated:: setting[" << target->getId() << "] to.done" << std::endl;
	boost::unique_lock<boost::mutex> lock(mutex_agentDone);
	if(REGISTERED_AGENTS.setDone(target,true))
	{
		Print() << "Agent[" << target->getId() << "] done" << std::endl;
//		duplicateEntityDoneChecker.insert(target);
		COND_VAR_AGENT_DONE.notify_all();
	}
	else
	{
		Print() << "Thread[" << boost::this_thread::get_id() << "] Discarded Agent[" << target->getId() << "]" << std::endl;
	}
}

void Broker::onClientRegister(
		sim_mob::event::EventId id,
		sim_mob::event::EventPublisher* sender,
		const ClientRegistrationEventArgs& argums) {

	ConfigParams::ClientType type = argums.getClientType();
	boost::shared_ptr<ClientHandler>clientHandler = argums.getClient();

	switch (type) {
	case ConfigParams::ANDROID_EMULATOR: {
		//if we are operating on android-ns3 set up,
		//each android client registration should be brought to
		//ns3's attention
		if (ConfigManager::GetInstance().FullConfig().getAndroidClientType()!= "android-ns3") {
			break;
		}
		//note: based on the current implementation of
		// the ns3 client registration handler, informing the
		//statistics of android clients IS a part of ns3
		//registration and configuration process. So the following implementation
		//should be executed for those android clients who will join AFTER
		//ns3's registration. So we check if the ns3 is already registered or not:
		if (clientList.find(ConfigParams::NS3_SIMULATOR) == clientList.end()) {
			break;
		}
		msg_header mHeader_("0", "SIMMOBILITY", "AGENTS_INFO", "SYS");
		Json::Value jMsg = JsonParser::createMessageHeader(mHeader_);
		const Agent *agent = clientHandler->agent;
		Json::Value jAgent;
		jAgent["AGENT_ID"] = agent->getId();
		//sorry it should be in an array to be compatible with the other
		//place where many agents are informed to be added
		Json::Value jArray_agent;
		jArray_agent.append(jAgent);
		//sorry again, compatibility issue. I will change this later.
		Json::Value jArray_add;
		jArray_add["ADD"].append(jArray_agent);
		insertSendBuffer(clientHandler->cnnHandler,jArray_add);
		break;
	}

	}
}

//todo: again this function is also intrusive to Broker. find a way to move the following switch cases outside the broker-vahid
//todo suggestion: for publishment, don't iterate through the list of clients, rather, iterate the publishers list, access their subscriber list and say publish and publish for their subscribers(keep the clientlist for MHing only)
void Broker::processPublishers(timeslice now) {
	PublisherList::pair publisher_pair;
	BOOST_FOREACH(publisher_pair, publishers)
	{
		//easy reading
		SIM_MOB_SERVICE service = publisher_pair.first;
		sim_mob::event::EventPublisher & publisher = *publisher_pair.second;

		switch (service) {
		case sim_mob::SIMMOB_SRV_TIME: {
			publisher.Publish(COMMEID_TIME, TimeEventArgs(now));
			break;
		}
		case sim_mob::SIMMOB_SRV_LOCATION: {

			//get to each client handler, look at his requred service and then publish for him
			ClientList::pair clientsByType;
			ClientList::IdPair clientsByID;
			BOOST_FOREACH(clientsByType, clientList)
			{
				BOOST_FOREACH(clientsByID, clientsByType.second)
				{
					boost::shared_ptr<sim_mob::ClientHandler> & cHandler = clientsByID.second;//easy read
					publisher.Publish(COMMEID_LOCATION,(void*) cHandler->agent,LocationEventArgs(cHandler->agent));
				}
			}
			break;
		}
		case sim_mob::SIMMOB_SRV_ALL_LOCATIONS: {
			publisher.Publish(COMMEID_LOCATION,(void*) COMMCID_ALL_LOCATIONS,AllLocationsEventArgs(REGISTERED_AGENTS));
			break;
		}
		default:
			break;
		}
	}
}

void Broker::sendReadyToReceive()
{
	ClientList::pair clientByType;
	ClientList::IdPair clientByID;

	boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
	msg_header msg_header_;
	{
		boost::unique_lock<boost::mutex> lock(mutex_clientList);
	BOOST_FOREACH(clientByType, clientList)
	{
		BOOST_FOREACH(clientByID, clientByType.second)
		{
			clnHandler = clientByID.second;
			msg_header_.msg_cat = "SYS";
			msg_header_.msg_type = "READY_TO_RECEIVE";
			msg_header_.sender_id = "0";
			msg_header_.sender_type = "SIMMOBILITY";
			Json::Value msg = JsonParser::createMessageHeader(msg_header_);
			insertSendBuffer(clnHandler->cnnHandler, msg);
		}
	}
	}
}

void Broker::processOutgoingData(timeslice now)
{
//	now send what you have to send:
	int debug_sendBuffer_cnt = sendBuffer.size();
	int debug_cnt = 0;
	int debug_buffer_size;
	Json::FastWriter debug_writer;
	std::ostringstream debug_out;
	for(SEND_BUFFER<Json::Value>::iterator it = sendBuffer.begin(); it!= sendBuffer.end(); it++, debug_cnt++)
	{
		sim_mob::BufferContainer<Json::Value> & buffer = it->second;
		boost::shared_ptr<sim_mob::ConnectionHandler> cnn = it->first;

		//build a jsoncpp structure comprising of a header and data array(containing messages)
		Json::Value jpacket;
		Json::Value jheader;
		Json::Value jpacketData;
		Json::Value jmsg;
		debug_buffer_size = buffer.size();
		jpacketData.clear();
		while(buffer.pop(jmsg))
		{
			jpacketData.append(jmsg);
		}
		int nof_messages;
		if(!(nof_messages = jpacketData.size()))
		{
			continue;
		}
		jheader = JsonParser::createPacketHeader(pckt_header(nof_messages));
		jpacket.clear();
		jpacket["PACKET_HEADER"] = jheader;
		jpacket["DATA"] = jpacketData;

		//convert the jsoncpp packet to a json string
		std::string str = Json::FastWriter().write(jpacket);
		cnn->async_send(str);
	}
	sendBuffer.clear();
}


//checks to see if the subscribed entity(agent) is alive
bool Broker::deadEntityCheck(sim_mob::AgentCommUtilityBase * info) {
	if(!info)
	{
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
		if(!managedEntities_.size())
		{
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
void Broker::refineSubscriptionList() {
	Print() << "inside Broker::refineSubscriptionList" << std::endl;
//	AgentsMap::iterator it, it_end(registeredAgents.end());
//	for(it = registeredAgents.begin(); it != it_end; it++)
//	{
//		sim_mob::Agent * target = (*it).first;
//		//you or your worker are probably dead already. you just don't know it
//		if (!target->currWorkerProvider) {
////			Print() << "1-refine subscription for agent ["  << target << "]" << std::endl;
//			unRegisterEntity(target);
//			continue;
//		}
//		const std::vector<sim_mob::Entity*> & managedEntities_ = (target->currWorkerProvider)->getEntities();
//		std::vector<sim_mob::Entity*>::const_iterator  it_entity = std::find(managedEntities_.begin(), managedEntities_.end(), target);
//		if(it_entity == managedEntities_.end())
//		{
////			Print() << "2-refine subscription for agent ["  << target << "]" << std::endl;
//			unRegisterEntity(target);
//			continue;
//		}
//		else
//		{
////			Print() << std::dec << "_Agent [" << target << ":" << *it_entity << "] is still among " << (int)((target->currWorker)->getEntities().size()) << " entities of worker[" << target->currWorker << "]" << std::endl;
//		}
//	}
	//do all the operation using the objects's mutex
	boost::function<void(sim_mob::Agent*)> Fn = boost::bind(&Broker::refineSubscriptionList, this,_1);
	REGISTERED_AGENTS.for_each_agent(Fn);
}

//tick waiting for them to finish
void Broker::refineSubscriptionList(sim_mob::Agent * target) {
		//you or your worker are probably dead already. you just don't know it
		if (!target->currWorkerProvider) {
			Print() << "1-refine subscription for agent ["  << target << "]" << std::endl;
			unRegisterEntity(target);
			return;
		}
		const std::set<sim_mob::Entity*> & managedEntities_ = (target->currWorkerProvider)->getEntities();
		std::set<sim_mob::Entity*>::const_iterator  it_entity = std::find(managedEntities_.begin(), managedEntities_.end(), target);
		if(it_entity == managedEntities_.end())
		{
			Print() << "2-refine subscription for agent ["  << target << "]" << std::endl;
			unRegisterEntity(target);
			return;
		}
		else
		{
//			Print() << std::dec << "_Agent [" << target << ":" << *it_entity << "] is still among " << (int)((target->currWorker)->getEntities().size()) << " entities of worker[" << target->currWorker << "]" << std::endl;
		}
}
//sim_mob::Broker sim_mob::Broker::instance(MtxStrat_Locked, 0);

//todo:  put a better condition here. this is just a placeholder
bool Broker::isWaitingForAgentRegistration() const
{
	bool res = agentBlockers.at(0)->calculateWaitStatus();
	if(res)
	{
//		Print() <<  "Evaluating Registered Agents: WAIT" << std::endl;;
//		Print() << "isWaitingForAgentRegistration is not qualified" << std::endl;
	}
	else
	{
//		Print() << "Evaluating Registered Agents: DONT_WAIT" << std::endl;
	}
	return res;
}
//todo:  put a better condition here. this is just a placeholder
bool Broker::clientsQualify() const
{
	return clientList.size() >= MIN_CLIENTS;
}

//returns true if you need to wait
bool Broker::isWaitingForAnyClientConnection() {
//	Print() << "inside isWaitingForAnyClientConnection " << clientBlockers.size() << std::endl;
	BrokerBlockers::IdPair pp;
	int i = -1;
	BOOST_FOREACH(pp, clientBlockers) {
		i++;
		if (pp.second->isWaiting()) {
			Print() << " isWaitingForAnyClientConnection[" << i << "] : wait" << std::endl;
			return true;
		}
	}
//	Print() << "isWaitingForAnyClientConnection : Dont wait" << std::endl;
	return false;
}

bool Broker::wait()
{
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

	int i = -1;
	{
		boost::unique_lock<boost::mutex> lock(mutex_client_request);
		while (!brokerCanTickForward) {
			Print() << ++i << " brokerCanTickForward->WAITING" << std::endl;
			COND_VAR_CLIENT_REQUEST.wait(lock);
			Print() << "COND_VAR_CLIENT_REQUEST released" << std::endl;
			processClientRegistrationRequests();

			bool res1 = isWaitingForAgentRegistration();
			bool res2 = isWaitingForAnyClientConnection();
			bool res3 = !res1 && !res2;
			bool res = /*brokerCanTickForward ||*/res3;
			brokerCanTickForward = res;
			Print() << "brokerCanTickForward["<< res1 << "-" << res2 << "]"  << std::endl;
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

void Broker::waitForAgentsUpdates()
{
	int i = 0;
//	boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::seconds(1);

	boost::unique_lock<boost::mutex> lock(mutex_agentDone);
//	refineSubscriptionList();
	while(!allAgentUpdatesDone()) {
		Print() << "waitForAgentsUpdates _WAIT" << std::endl;
//		COND_VAR_AGENT_DONE.timed_wait(lock, timeout);
		COND_VAR_AGENT_DONE.wait(lock);
		Print() << "waitForAgentsUpdates _WAIT_released" << std::endl;
//		refineSubscriptionList();
	}
}
bool Broker::isClientDone(boost::shared_ptr<sim_mob::ClientHandler> &clnHandler)
{
	if(clientDoneChecker.end() == clientDoneChecker.find(clnHandler->cnnHandler))
	{
		return false;
	}
	return true;
}
bool Broker::allClientsAreDone()
{
		ClientList::pair clientByType;
		ClientList::IdPair clientByID;

		boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
		msg_header msg_header_;
		boost::unique_lock<boost::mutex> lock(mutex_clientList);
		BOOST_FOREACH(clientByType, clientList)
		{
			BOOST_FOREACH(clientByID, clientByType.second)
			{
				clnHandler = clientByID.second;

//				Print()
//					<< "\nBroker::allClientsAreDone analyzing client["
//					<< clnHandler->client_type << ":" << clnHandler->clientID << "]" << std::endl;

				if(!clnHandler)
				{
//					Print() << "1 Not valid" << std::endl;
					continue;
				}
				if(!(clnHandler->cnnHandler))
				{
//					Print() << "2 Not valid" << std::endl;
					continue;
				}
				if(!(clnHandler->cnnHandler->is_open()))
				{
//					Print() << "3 Not open" << std::endl;
					continue;
				}
				if(!(clnHandler->isValid()))
				{
//					Print() << "4 Not valid" << std::endl;
					continue;
				}
				if(!(clnHandler->cnnHandler->isValid()))
				{
//					Print() << "5 Not valid" << std::endl;
					continue;
				}
				//but
				if(!isClientDone(clnHandler))
				{
//					Print() << "Not Done yet" << std::endl;
					return false;
				}
			}
		}
//		Print()<< "Broker::allClientsAreDone --true" << std::endl;
		return true;
}

Entity::UpdateStatus Broker::update(timeslice now)
{
	Print() << "Broker tick:"<< now.frame() << std::endl;
	//step-1 : open the door to ouside world, //transfer this to frame_init
	if(now.frame() == 0) {
		connection->start();
	}
	Print() << "=====================ConnectionStarted =======================================" << std::endl;
	//Step-2: Ensure that we have enough clients to process
	//(in terms of client type (like ns3, android emulator, etc) and quantity(like enough number of android clients) ).
	//Block the simulation here(if you have to)
		wait();
//		Print() << "Broker NOT Blocking" << std::endl;
	Print() << "===================== wait Done =======================================" << std::endl;

	//step-3: Process what has been received in your receive container(message queue perhaps)
	processIncomingData(now);
	Print() << "===================== processIncomingData Done =======================================" << std::endl;
	//step-4: if need be, wait for all agents(or others)
	//to complete their tick so that you are the last one ticking)
	waitForAgentsUpdates();
	Print() << "===================== waitForAgentsUpdates Done =======================================" << std::endl;
	//step-5: signal the publishers to publish their data
	processPublishers(now);
	Print() << "===================== processPublishers Done =======================================" << std::endl;
//	step-5.5:for each client, append a message at the end of all messages saying Broker is ready to receive your messages
	sendReadyToReceive();
//	Print() << "===================== sendReadyToReceive Done =======================================" << std::endl;
	//step-6: Now send all what has been prepared, by different sources, to their corresponding destications(clients)
	processOutgoingData(now);
	Print() << "===================== processOutgoingData Done =======================================" << std::endl;
	//step-7:
	//the clients will now send whatever they want to send(into the incoming messagequeue)
	//followed by a Done! message.That is when Broker can go forwardClientList::pair clientByType;
	waitForClientsDone();

//	//Step 7.5: output
//	if (ConfigManager::GetInstance().FullConfig().ProfileWorkerUpdates()) {
//		//TODO: This is a bit of a hack; won't work with both ns3/Android enabled. ~Seth.
//		size_t sz = 0;
//		for (ClientList::type::iterator it=clientList.begin(); it!=clientList.end(); it++) {
//			sz += it->second.size();
//		}
//		std::stringstream msg;
//		msg <<sz;
//		profile->logAgentCustomMessage(*this, now, "custom-num-connected-clients", msg.str());
//		//TODO: End hack.
//
//		//TODO: And another hack!
//		const uint32_t TickAmt = 100; //"Every X ticks"
//		const uint32_t TickStep = 1; //HACK: For what the Worker sees (for all Agents).
//		if ((now.frame()/TickStep)%TickAmt==0) {
//			profile->flushLogFile();
//		}
//	}


	Print() << "===================== waitForClientsDone Done =======================================" << std::endl;
	//step-8: final steps that should be taken before leaving the tick
	//prepare for next tick.
	cleanup();//
//	Print() << "===================== cleanup Done =======================================" << std::endl;
//	return proudly
	return UpdateStatus(UpdateStatus::RS_CONTINUE);

}

void Broker::removeClient(ClientList::iterator it_erase)
{
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
void Broker::waitForClientsDone()
{
	boost::unique_lock<boost::mutex> lock(mutex_clientDone);
	while(!allClientsAreDone())
	{
//		Print() << "Broker::waitForClientsDone-WAIT" << std::endl;
		COND_VAR_CLIENT_DONE.wait(lock);
//		Print() << "Broker::waitForClientsDone-NOT WAIT" << std::endl;

	}
}
void Broker::cleanup()
{
	//for internal use
	duplicateEntityDoneChecker.clear();
	clientDoneChecker.clear();
	return;

	//note:this part is supposed to delete clientList entries for the dead agents
	//But there is a complication on deleting connection handler
	//there is a chance the sockets are deleted before a send ACK arrives

}

std::map<std::string, sim_mob::Broker*> & Broker::getExternalCommunicators()  {
	return externalCommunicators;
}

sim_mob::Broker* Broker::getExternalCommunicator(const std::string & value) {
	if(externalCommunicators.find(value) != externalCommunicators.end())
	{
		return externalCommunicators[value];
	}
	return 0;
}

void Broker::addExternalCommunicator(const std::string & name, sim_mob::Broker* broker){
	externalCommunicators.insert(std::make_pair(name,broker));
}

//abstracts & virtuals
void Broker::load(const std::map<std::string, std::string>& configProps){};
void Broker::frame_output(timeslice now){};
bool Broker::isNonspatial(){
	return true;
};
}//namespace

