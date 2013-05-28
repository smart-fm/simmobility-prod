#include "Broker.hpp"
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <json/json.h>
#include "entities/AuraManager.hpp"
#include "workers/Worker.hpp"
#include "entities/commsim/communicator/message/derived/roadrunner/RR_Factory.hpp"

namespace sim_mob
{
void Broker::enable() { enabled = true; }
void Broker::disable() { enabled = false; }
bool Broker::isEnabled() const { return enabled; }
void Broker::insertSendBuffer(DataElement& value)
{
	sendBuffer.add(value);
}
Broker::Broker(const MutexStrategy& mtxStrat, int id )
: Agent(mtxStrat, id)
,enabled(true) //If a Broker is created, we assume it is enabled.
{
	//Various Initializations
	Broker_Client_Mutex.reset(new boost::mutex);
	COND_VAR_CLIENT_REQUEST.reset(new boost::condition_variable);
	connection.reset(new ConnectionServer(clientRegistrationWaitingList, Broker_Client_Mutex,COND_VAR_CLIENT_REQUEST));
	 Broker_Mutex.reset(new boost::shared_mutex);
	 Broker_Mutex_Send.reset(new boost::shared_mutex);
	 Broker_Mutex_Receive.reset(new boost::shared_mutex);
	 mutex_collection.push_back(Broker_Mutex);
	 mutex_collection.push_back(Broker_Mutex_Send);
	 mutex_collection.push_back(Broker_Mutex_Receive);
	 sendBuffer.setOwnerMutex(Broker_Mutex_Send);
	 brokerInOperation = false;

	 //todo, for the following maps , think of something non intrusive to broker. This is merely hardcoding-vahid
	 //publishers
	 publishers.insert(std::make_pair(SIMMOB_SRV_LOCATION, boost::shared_ptr<sim_mob::Publisher> (new sim_mob::LocationPublisher())));
	 publishers.insert(std::make_pair(SIMMOB_SRV_TIME, boost::shared_ptr<sim_mob::Publisher> (new sim_mob::TimePublisher())));
	 //current message factory
	 boost::shared_ptr<sim_mob::MessageFactory<msg_ptr, std::string> > factory(new sim_mob::roadrunner::RR_Factory() );
	 //note that both client types refer to the same message factory belonging to roadrunner application. we will modify this to a more generic approach later-vahid
	 messageFactories.insert(std::make_pair(ANDROID_EMULATOR, factory) );
	 messageFactories.insert(std::make_pair(NS3_SIMULATOR, factory) );
}


Broker::~Broker()
{
}
/*
 * the callback function called by connection handlers to ask
 * the Broker what to do when a message arrives(this method is to be called
 * after whoareyou, registration and connectionHandler creation....)
 */
void Broker::messageReceiveCallback(ConnectionHandler& cnnHandler, std::string input)
{
	//extract messages from the input string(remember that a message object carries a reference to its handler also)
	std::vector<msg_ptr> messages(messageFactories[cnnHandler.clientType]->createMessage(input));

//	post the messages into the message queue one by one(add their cnnHandler also)
	for(std::vector<msg_ptr>::iterator it = messages.begin(); it != messages.end(); it++)
	{
		receiveQueue.post(boost::make_tuple(cnnHandler,*it));
	}
}

boost::shared_ptr<boost::mutex>  Broker::getBrokerMutex()
{
	return Broker_Mutex;
}
 boost::shared_ptr<boost::shared_mutex> Broker::getBrokerMutexSend()
 {
 	return Broker_Mutex_Send;
 }
boost::shared_ptr<boost::shared_mutex> Broker::getBrokerMutexReceive()
{
	return Broker_Mutex_Receive;
}
AgentsMap & Broker::getRegisteredAgents() {
	return registeredAgents;
}
ClientWaitList & Broker::getClientWaitingList(){
	return clientRegistrationWaitingList;
}
ClientList & Broker::getClientList(){
	return clientList;
}
PublisherList & Broker:: getPublishers()
{
	return publishers;
}

void Broker::processClientRegistrationRequests()
{
	boost::unique_lock<boost::mutex> lock(*Broker_Mutex);
	for(ClientWaitList::iterator it = clientRegistrationWaitingList.begin(), it_end(clientRegistrationWaitingList.end()); it != it_end;  it++ )
	{
		boost::shared_ptr<ClientRegistrationHandler > handler = clientRegistrationFactory.getHandler((ClientType)(it->first));
		if(handler->handle(*this,it->second))
		{
			//success: handle() just added to the client to the main client list and started its connectionHandler
			//get this request out of registration list.
			it = clientRegistrationWaitingList.erase(it) ;
		}
	}
}

bool  Broker::subscribeEntity(sim_mob::JCommunicationSupport* value)
{
	registeredAgents.insert(std::make_pair(value->getEntity(), value));
	return true;
}

void  Broker::unRegisterEntity(sim_mob::JCommunicationSupport &value)
{
	unRegisterEntity(&value.getEntity());
}

void  Broker::unRegisterEntity(const sim_mob::Agent * agent)
{
	//search agent's list looking for this agent
	registeredAgents.erase(agent); //hopefully the agent is there
	//search the internal container also
	duplicateEntityDoneChecker.erase(agent);

	//search registered clients list looking for this agent. whoever has it, dump him
	std::pair<unsigned int , registeredClient> client;
	for(ClientList::iterator it = clientList.begin(); it != clientList.end(); it++)
	{
		if(it->second.agent == agent)
		{
			it = clientList.erase(it);
		}
	}
}

void Broker::processIncomingData(timeslice now)
{
	//just pop off the message queue and click handl ;)
	MessageElement msgTuple;
	while(receiveQueue.pop(msgTuple))
	{
		msg_ptr &msg = msgTuple.get<1>();
		msg->supplyHandler()->handle(msg);
	}
}

bool Broker::frame_init(timeslice now){
//	Print() << "Broker ConnectionServer Starting" << std::endl;
//
//	Print() << "Broker ConnectionServer Started" << std::endl;
//	Print() << "BTW here are the mutexes :" <<
//			"Broker_Mutex[" << Broker_Mutex <<
//			"] \n Broker_Mutex_Send[" << Broker_Mutex_Send <<
//			"] \n Broker_Mutex_Receive[" << Broker_Mutex_Receive <<
//			"]" << std::endl;
	return true;
};
Entity::UpdateStatus Broker::frame_tick(timeslice now){
	return Entity::UpdateStatus::Continue;
}

bool Broker::allAgentUpdatesDone()
{
//	boost::unique_lock< boost::shared_mutex > lock(*NS3_Communicator_Mutex);
	AgentsMap::iterator it = registeredAgents.begin(), it_end(registeredAgents.end());

	for(; it != it_end; it++)
	{

			sim_mob::JCommunicationSupport &info = *(it->second);//easy read
			try
			{
				if (info.isAgentUpdateDone())
				{
					boost::unique_lock< boost::shared_mutex > lock(*Broker_Mutex);
					duplicateEntityDoneChecker.insert(it->first);
				}
			}
			catch(std::exception& e)
			{
				Print() << "Exception Occured " << e.what() << std::endl;
				if(deadEntityCheck(info))
				{
					unRegisterEntity(info);
				}
				else
					throw std::runtime_error("Unknown Error checking entity");
			}
	}
	return(duplicateEntityDoneChecker.size() >= registeredAgents.size());
}

void Broker::processPublishers(timeslice now) {
	std::pair<SIM_MOB_SERVICE, boost::shared_ptr<sim_mob::EventPublisher> > publisher;
	std::pair<unsigned int, registeredClient> client;
	//iterate through each registered client
	BOOST_FOREACH(client, clientList) {
		//get the services client is subscribed to
		std::set<SIM_MOB_SERVICE> & requiredServices =
				client.second.requiredServices;
		SIM_MOB_SERVICE service;
		//iterate through the client's set of services
		BOOST_FOREACH(service, requiredServices) {
			//a small check
			if (publishers.find(service) == publishers.end()) {
				continue;
			}
			//find a publisher
			boost::shared_ptr<sim_mob::Publisher> publisher = publishers[service];
			switch (publisher->myService) {
			case sim_mob::SIMMOB_SRV_TIME: {
				publisher->Publish(COMMEID_TIME, TimeEventArgs(timeslice));
				break;
			}
			case sim_mob::SIMMOB_SRV_LOCATION: {
				publisher->Publish(COMMEID_LOCATION,
						LocationEventArgs(client.second.agent));
				break;
			}
			}
		}
	}
}
void Broker::processOutgoingData(timeslice now)
{
//	now send what you have to send:
		DataElement dataElement ;
		while(sendBuffer.pop(dataElement))
		{
			ConnectionHandler *cnn = dataElement.get<1>();
			std::string message =  dataElement.get<2>();
			cnn->send(message);
		}
}

//checks to see if the subscribed entity(agent) is alive
bool Broker::deadEntityCheck(sim_mob::JCommunicationSupport & info) {
//	//some top notch optimizasion! to check if the agent is alive at all?
//	info.cnt_1++;
//	if (info.cnt_1 < 1000)
//		return false;
	//you or your worker are probably dead already. you just don't know it
	try {

		if (!(info.getEntity().currWorker)) {
			Print() << "currWorker dead" << std::endl;
			return true;
		}

		//one more check to see if the entity is deleted
		const std::vector<sim_mob::Entity*> & managedEntities_ =
				info.getEntity().currWorker->getEntities();
		std::vector<sim_mob::Entity*>::const_iterator it =
				managedEntities_.begin();
		if(!managedEntities_.size())
		{
			return true;
		}
		for (std::vector<sim_mob::Entity*>::const_iterator it =
				managedEntities_.begin(); it != managedEntities_.end(); it++) {
			//agent is still being managed, so it is not dead
			if (*it == &(info.getEntity()))
				return false;
		}
	} catch (std::exception& e) {
		return true;
	}

	return true;
}

//iterate the entire subscription list looking for
//those who are not done with their update and check if they are dead.
//you better hope they are dead otherwise you have to hold the simulation
//tick waiting for them to finish
void Broker::refineSubscriptionList() {
	AgentsMap::iterator it, it_end(registeredAgents.end());
	for(it = registeredAgents.begin(); it != it_end; it++)
	{
		const sim_mob::Agent * target = (*it).first;
		//you or your worker are probably dead already. you just don't know it
		if (!target->currWorker)
			{
				unRegisterEntity(target);
				continue;
			}
		const std::vector<sim_mob::Entity*> & managedEntities_ = (target->currWorker)->getEntities();
		std::vector<sim_mob::Entity*>::const_iterator  it_entity = std::find(managedEntities_.begin(), managedEntities_.end(), target);
		if(it_entity == managedEntities_.end())
		{
			unRegisterEntity(target);
			continue;
		}
		else
		{
//			Print() << std::dec << "_Agent [" << target << ":" << *it_entity << "] is still among " << (int)((target->currWorker)->getEntities().size()) << " entities of worker[" << target->currWorker << "]" << std::endl;
		}
	}
}
//sim_mob::Broker sim_mob::Broker::instance(MtxStrat_Locked, 0);

//todo:  put a better condition here. this is just a placeholder
bool Broker::subscriptionsQualify() const
{
	return clientList.size() >= MIN_CLIENTS;
}
//todo:  put a better condition here. this is just a placeholder
bool Broker::clientsQualify() const
{
	return clientList.size() >= MIN_CLIENTS;
}

bool Broker::waitForClients()
{
	boost::unique_lock<boost::mutex> lock(*Broker_Client_Mutex);

	/**if:
	 * 1- number of subscribers is too low
	 * 2-there is no client(emulator) waiting in the queue
	 * 3-this update function never started to process any data so far
	 * then:
	 *  wait for enough number of clients and agents to join
	 */
	while(!(subscriptionsQualify() && clientsQualify() && brokerInOperation)) {
		COND_VAR_CLIENT_REQUEST->wait(lock);
		processClientRegistrationRequests();
//		processEntityWaitingList();
	}

	//now that you are qualified(have enough number of subscribers), then
	//you are considered to have already started operating.
	if(subscriptionsQualify()) {
		brokerInOperation = true;
	}

	//broker started before but is no more qualified to run
	if(brokerInOperation && (!subscriptionsQualify())) {
		//don't block, just cooperate & don't do anything until this simulation ends
		//TODO: This might be why our client eventually gives up.
		return false;
	}

	//Success! Continue.
	return true;
}

void Broker::waitForUpdates()
{
	while(!allAgentUpdatesDone()) {
		refineSubscriptionList();
	}
}

Entity::UpdateStatus Broker::update(timeslice now)
{
	Print() << "Broker tick:"<< now.frame() << "\n";
	//step-1 : open the door to ouside world
	if(now.frame() == 0) {
		connection->start();
	}

	//Step-2: Ensure that we have enough clients to process
	//(in terms of client type (like ns3, android emulator, etc) and quantity(like enough number of android clients) ).
	//Block the simulation here(if you have to)
	if (!waitForClients()) {
		return UpdateStatus(UpdateStatus::RS_CONTINUE);
	}
	//step-3: Process what has been received in your receive container(message queue perhaps)
	processIncomingData(now);
	//step-4: if need be, wait for all agents(or others)
	//to complete their tick so that you are the last one ticking)
	waitForUpdates();
	//step-5: signal the publishers to publish their data
	processPublishers(now);

	//step-6: Now send what has been prepared by different sources to their corresponding destications(clients)
	processOutgoingData(now);

	//step-7: final steps that should be taken before leaving the tick
	//this method may have already been called through waitForClients()
	//we just see if anything is left off
	processClientRegistrationRequests();
	//prepare for next tick.
	duplicateEntityDoneChecker.clear();
//	return proudly
	return UpdateStatus(UpdateStatus::RS_CONTINUE);

}

//void Broker::unicast(const sim_mob::Agent * agent, std::string data)
//{
//	//find the connection handler
//	agentIterator it = agentSubscribers_.find(agent);
//	if(it == agentSubscribers_.end()) {
//		Print() << "unicast Failed. Agent not Found" << std::endl;
//	}
////	ConnectionHandler handler = it->handler;
//	it->handler->send(data);
//}

}//namespace



























//data is : {"messageType":"ANNOUNCE", "ANNOUNCE" : {"Sender":"clientIdxxx", "x":"346378" , "y":"734689237", "OfferingTokens":["A", "B", "C"]}}
//no need to parse the message much:
////just update its x,y location, then forward it to nearby agents
//bool Broker::handleANNOUNCE(std::string data)
//{
//	Json::Value root;
//	Json::Reader reader;
//	bool parsedSuccess = reader.parse(data, root, false);
//	if(not parsedSuccess)
//	{
//		Print() << "Parsing [" << data << "] Failed" << std::endl;
//		return false;
//	}
//	Json::Value body = root["ANNOUNCE"];
//	unsigned int clientID = body["Sender"].asUInt();
//	//get the mapping agent
//	clientIterator it = clientSubscribers_.find(clientID);
//	const Agent * agent = it->agent;
//	//get agent's fresh position (to be used in the generated message too)
//	int x,y;
//	x = agent->xPos.get();
//	y = agent->yPos.get();
//	//update location
//	body["x"] = x;
//	body["y"] = y;
//	root["ANNOUNCE"] = body;//save back
//	//now create a fresh json string
//	Json::FastWriter writer;
//	std::string output = writer.write(root);
//	//get nearby agents
//	sim_mob::Point2D lower(x-3500, y-3500);
//	sim_mob::Point2D upper(x+3500, y+3500);
//	//get nearby agents
//	std::vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(lower,upper);
//	for(std::vector<const Agent*>::iterator it = nearby_agents.begin(); it != nearby_agents.end(); it++)
//	{
//		sendBufferMap[*it].add(BufferContainer::makeDataElement(output,clientID,(unsigned long)(*it)));
//	}
//	return true;
//}
//
////data is : {"messageType":"KEY_REQUEST", "KEY_REQUEST" : {"Sender":"clientIdxxx", "Receiver" : "clientIdyyy", "RequestingTokens":["A", "B", "C"]}}
////just extract the receiver and forward the string to it without modifications
//bool sim_mob::Broker::handleKEY_REQUEST(std::string data)
//{
//	//parse
//	Json::Value root;
//	Json::Reader reader;
//	bool parsedSuccess = reader.parse(data, root, false);
//	if(not parsedSuccess)
//	{
//		Print() << "Parsing [" << data << "] Failed" << std::endl;
//		return false;
//	}
//	//get the destination
//	Json::Value body = root["KEY_REQUEST"];
//	unsigned int receivingClientID = body["Receiver"].asUInt();
//	unsigned int sendingClientID = body["Sender"].asUInt();
//	clientIterator it = clientSubscribers_.find(receivingClientID);
//	if(it == clientSubscribers_.end())
//	{
//		return false;
//	}
//	const sim_mob::Agent* receiver = it->agent;
//	sendBufferMap[receiver].add(BufferContainer::makeDataElement(data,sendingClientID,receivingClientID));
//	return true;
//}
//
////data is : {"messageType":"KEY_SEND", "KEY_SEND" : {"Sender":"clientIdxxx", "Receiver" : "clientIdyyy", "SendingTokens":["A", "B", "C"]}}
////just extract the receiver and forward the string to it without modifications
//bool sim_mob::Broker::handleKEY_SEND(std::string data)
//{
//	//parse
//	Json::Value root;
//	Json::Reader reader;
//	bool parsedSuccess = reader.parse(data, root, false);
//	if(not parsedSuccess)
//	{
//		Print() << "Parsing [" << data << "] Failed" << std::endl;
//		return false;
//	}
//	//get the destination
//	Json::Value body = root["KEY_SEND"];
//	unsigned int receivingClientID = body["Receiver"].asUInt();
//	unsigned int sendingClientID = body["Sender"].asUInt();
//	clientIterator it = clientSubscribers_.find(receivingClientID);
//	if(it == clientSubscribers_.end())
//	{
//		return false;
//	}
//	const sim_mob::Agent* receiver = it->agent;
//	sendBufferMap[receiver].add(BufferContainer::makeDataElement(data,sendingClientID,receivingClientID));
//	return true;
//}
