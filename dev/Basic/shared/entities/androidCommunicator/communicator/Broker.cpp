#include "Broker.hpp"
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <jsoncpp/json.h>
#include "Message/RoadRunnerHandler.hpp"
#include "entities/AuraManager.hpp"
#include "workers/Worker.hpp"

namespace sim_mob
{
void Broker::enable() { enabled = true; }
void Broker::disable() { enabled = false; }
bool Broker::isEnabled() { return enabled; }
//void Broker::io_service_run(boost::shared_ptr<boost::asio::io_service> io_service_)
//{
//	server_.start();
//	io_service_->run();
//}
sim_mob::BufferContainer &Broker::getSendBuffer()
{
	//		boost::unique_lock< boost::shared_mutex > lock(*NS3_Communicator_Mutex);
	return sendBuffer;
}
subscriptionC &Broker::getSubscriptionList() { return subscriptionList;}
Broker::Broker(const MutexStrategy& mtxStrat, int id )
: Agent(mtxStrat, id),/*io_service_(new boost::asio::io_service()),*/
  server_(clientList)
,subscriberList_(get<0>(getSubscriptionList()))
,agentSubscribers_(get<1>(getSubscriptionList()))
,clientSubscribers_(get<2>(getSubscriptionList()))
{
	 MessageMap = boost::assign::map_list_of("ANNOUNCE", ANNOUNCE)("KEY_REQUEST", KEY_REQUEST)("KEY_SEND",KEY_SEND);

	 Broker_Mutex.reset(new boost::shared_mutex);
	 Broker_Mutex_Send.reset(new boost::shared_mutex);
	 Broker_Mutex_Receive.reset(new boost::shared_mutex);
	 sendBuffer.setOwnerMutex(Broker_Mutex_Send);
	 receiveBuffer.setOwnerMutex(Broker_Mutex_Receive);
	 std::cout << "Broker Server Starting" << std::endl;
}

void Broker::start()
{

}

Broker::~Broker()
{
}

//called when a message is received in a connectionHandler
//so far there is no need to specify wich connection it is
//coz it is currently assumed that the string contains all the information
void Broker::handleReceiveMessage(std::string str){
	receiveBuffer.add(sim_mob::BufferContainer::makeDataElement(str));
}
void Broker::clientEntityAssociation(subscription subscription_)
{
	//Variable declaration
			std::pair<unsigned int,sim_mob::session_ptr > availableClient;
			boost::shared_ptr<session> session_;
			unsigned int clientID_;
			//poping from client list
			availableClient = clientList.front();
			clientList.pop();
			//filling temp data
			clientID_ = availableClient.first;
			session_ = availableClient.second;
			//filling real data
			subscription_.clientID = clientID_;
			subscription_.handler.reset(
					new sim_mob::ConnectionHandler(
							session_,
							*this,
							&Broker::handleReceiveMessage,
							clientID_,
							(unsigned long)const_cast<sim_mob::Agent*>(&(subscription_.JCommunicationSupport_->getEntity()))
							)
			);
			//inserting to list
			subscriberList_.insert(subscriberList_.begin(),subscription_);
			subscription_.JCommunicationSupport_->setMutexes(mutex_collection);
			//tell the agent he is subscribed
			CALL_MEMBER_FN(*(subscription_.JCommunicationSupport_), subscription_.JCommunicationSupport_->subscriptionCallback)(true);
			//start send/receive functionality right here. It seems there is no need of a thread
			subscription_.handler->start();
}

bool Broker::processEntityWaitingList()
{
	bool success = false;

	while(clientList.size() && agentWaitingList.size())
	{
		subscription subscription_ = agentWaitingList.begin()->second;
		agentWaitingList.erase(agentWaitingList.begin());
		clientEntityAssociation(subscription_);//client list will be reduced in this function, so no worries.
	}
}

void Broker::addAgentToWaitingList(sim_mob::JCommunicationSupport & value, subscription &subscription_)
{

	//no client(android emulator for now) available, putting into the waiting list
	agentWaitingList.at(&value.getEntity()) = subscription_;
	CALL_MEMBER_FN(value, value.subscriptionCallback)(false);
}
bool  Broker::subscribeEntity(sim_mob::JCommunicationSupport & value)
{
	bool success = false;
	subscription subscription_(&value);

	if(clientList.size())
	{
		clientEntityAssociation(subscription_);
		success = true;
	}
	else
	{
		addAgentToWaitingList(value, subscription_);
		success = false;
	}
	 return success;
}

bool  Broker::unSubscribeEntity(sim_mob::JCommunicationSupport &value)
{
//	boost::unique_lock< boost::shared_mutex > lock(*NS3_Communicator_Mutex);
	return unSubscribeEntity(&value.getEntity());
}

bool  Broker::unSubscribeEntity(const sim_mob::Agent * agent)
{

	agentSubscribers_.erase(agent);
//	boost::unique_lock< boost::shared_mutex > lock(*Broker_Mutex);
	//also refine duplicateEntityDoneChecker
	std::set<const sim_mob::Agent*>::iterator it = duplicateEntityDoneChecker.find(agent);
	if(it != duplicateEntityDoneChecker.end() ) {
		duplicateEntityDoneChecker.erase(it);
	}
	return false;//for now. I dont know if ne1 needs its return value
}

//Regular data that needs to be sent every tick
//todo: we need a more generic approach than this.
//this is more like a demo preparation
void Broker::preparePerTickData(timeslice now)
{
	//what time is it please?
	std::string time = sim_mob::JsonParser::makeTimeData(now.frame());
	subscriberIterator it = subscriberList_.begin(), it_end = subscriberList_.end();

	for(; it != it_end; it++)
	{
		//send time
		it->handler->send(time);
		//send location
		//1.get the agent
		const Entity * entity = it->agent;
		const Agent * agent = dynamic_cast<const Agent *>(entity);
		//2.get x,y
		int x,y;
		x = agent->xPos.get();
		y = agent->yPos.get();
		//3.convert to json
		std::string location_ = sim_mob::JsonParser::makeLocationData(x,y);

		//finally add both time and location to the corresponding agent's send buffer
		sendBufferMap[agent].add(BufferContainer::makeDataElement(time,0,(unsigned long)(entity)));
		sendBufferMap[agent].add(BufferContainer::makeDataElement(location_,0,(unsigned long)(entity)));
	}
}
void Broker::processIncomingData(timeslice now)
{
	DataElement dataElement;
	while(receiveBuffer.pop(dataElement))
	{

	//get the message received from client
	std::string message = boost::get<2>(dataElement);
	std::string type, data;
	Json::Value root_;
	sim_mob::JsonParser::getMessageTypeAndData(message, type, data, root_);
	switch(MessageMap[type])
	{
	case ANNOUNCE:
		//{"messageType":"ANNOUNCE", "ANNOUNCE" : {"Sender":"clientIdxxx", "x":"346378" , "y":"734689237", "OfferingTokens":["A", "B", "C"]}}
		//no need to parse the message much:
		//just update its x,y location, then forward it to nearby agents
		handleANNOUNCE(message);
		break;
	case KEY_REQUEST:
		//data is : {"messageType":"KEY_REQUEST", "KEY_REQUEST" : {"Sender":"clientIdxxx", "Receiver" : "clientIdyyy", "RequestingTokens":["A", "B", "C"]}}
		//just extract the receiver and forward the string to it without modifications
		handleKEY_REQUEST(message);
		break;
	case KEY_SEND:
		break;
	}
	}


}
bool Broker::frame_init(timeslice now){
//	std::cout << "Broker Server Starting" << std::endl;
//
//	std::cout << "Broker Server Started" << std::endl;
//	std::cout << "BTW here are the mutexes :" <<
//			"Broker_Mutex[" << Broker_Mutex <<
//			"] \n Broker_Mutex_Send[" << Broker_Mutex_Send <<
//			"] \n Broker_Mutex_Receive[" << Broker_Mutex_Receive <<
//			"]" << std::endl;

};

//data is : {"messageType":"ANNOUNCE", "ANNOUNCE" : {"Sender":"clientIdxxx", "x":"346378" , "y":"734689237", "OfferingTokens":["A", "B", "C"]}}
//no need to parse the message much:
//just update its x,y location, then forward it to nearby agents
bool Broker::handleANNOUNCE(std::string data)
{
	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(data, root, false);
	if(not parsedSuccess)
	{
		std::cout << "Parsing [" << data << "] Failed" << std::endl;
		return false;
	}
	Json::Value body = root["ANNOUNCE"];
	unsigned int clientID = body["Sender"].asUInt();
	//get the mapping agent
	clientIterator it = clientSubscribers_.find(clientID);
	const Agent * agent = it->agent;
	//get agent's fresh position (to be used in the generated message too)
	int x,y;
	x = agent->xPos.get();
	y = agent->yPos.get();
	//update location
	body["x"] = x;
	body["y"] = y;
	root["ANNOUNCE"] = body;//save back
	//now create a fresh json string
	Json::FastWriter writer;
	std::string output = writer.write(root);
	//get nearby agents
	sim_mob::Point2D lower(x-3500, y-3500);
	sim_mob::Point2D upper(x+3500, y+3500);
	//get nearby agents
	std::vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(lower,upper);
	for(std::vector<const Agent*>::iterator it = nearby_agents.begin(); it != nearby_agents.end(); it++)
	{
		sendBufferMap[*it].add(BufferContainer::makeDataElement(output,clientID,(unsigned long)(*it)));
	}
}

//data is : {"messageType":"KEY_REQUEST", "KEY_REQUEST" : {"Sender":"clientIdxxx", "Receiver" : "clientIdyyy", "RequestingTokens":["A", "B", "C"]}}
//just extract the receiver and forward the string to it without modifications
bool sim_mob::Broker::handleKEY_REQUEST(std::string data)
{
	//parse
	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(data, root, false);
	if(not parsedSuccess)
	{
		std::cout << "Parsing [" << data << "] Failed" << std::endl;
		return false;
	}
	//get the destination
	Json::Value body = root["KEY_REQUEST"];
	unsigned int receivingClientID = body["Receiver"].asUInt();
	unsigned int sendingClientID = body["Sender"].asUInt();
	clientIterator it = clientSubscribers_.find(receivingClientID);
	if(it == clientSubscribers_.end())
	{
		return false;
	}
	const sim_mob::Agent* receiver = it->agent;
	sendBufferMap[receiver].add(BufferContainer::makeDataElement(data,sendingClientID,receivingClientID));
}

//data is : {"messageType":"KEY_SEND", "KEY_SEND" : {"Sender":"clientIdxxx", "Receiver" : "clientIdyyy", "SendingTokens":["A", "B", "C"]}}
//just extract the receiver and forward the string to it without modifications
bool sim_mob::Broker::handleKEY_SEND(std::string data)
{
	//parse
	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(data, root, false);
	if(not parsedSuccess)
	{
		std::cout << "Parsing [" << data << "] Failed" << std::endl;
		return false;
	}
	//get the destination
	Json::Value body = root["KEY_SEND"];
	unsigned int receivingClientID = body["Receiver"].asUInt();
	unsigned int sendingClientID = body["Sender"].asUInt();
	clientIterator it = clientSubscribers_.find(receivingClientID);
	if(it == clientSubscribers_.end())
	{
		return false;
	}
	const sim_mob::Agent* receiver = it->agent;
	sendBufferMap[receiver].add(BufferContainer::makeDataElement(data,sendingClientID,receivingClientID));
}


bool Broker::allAgentUpdatesDone()
{
//	boost::unique_lock< boost::shared_mutex > lock(*NS3_Communicator_Mutex);
	subscriberIterator it = subscriberList_.begin(), it_end(subscriberList_.end());

	for(; it != it_end; it++)
	{

			sim_mob::JCommunicationSupport &info = *(it->JCommunicationSupport_);//easy read
//			if(deadEntityCheck(info))
//			{
//				throw std::runtime_error("You are dealing with a probably dead entity");
//			}
			try
			{
				if (info.isAgentUpdateDone())
				{
					boost::unique_lock< boost::shared_mutex > lock(*Broker_Mutex);
					duplicateEntityDoneChecker.insert(it->agent);
				}
			}
			catch(char * str)
			{
				if(deadEntityCheck(info))
				{
					unSubscribeEntity(info);
				}
				else
					throw std::runtime_error("Unknown Error checking entity");
			}
	}
	return(duplicateEntityDoneChecker.size() >= subscriberList_.size());
}

void Broker::processOutgoingData(timeslice now)
{
//	now send what you have to send:
	duplicateEntityDoneChecker.clear();
	std::map<const sim_mob::Agent *, sim_mob::BufferContainer >::iterator it = sendBufferMap.begin(),
			it_end = sendBufferMap.end();
	while(it!=it_end)
	{
		sim_mob::BufferContainer &sendBuffer = it->second;
		DataElement dataElement;
		while(sendBuffer.pop(dataElement))
		{
			agentIterator it_agent = agentSubscribers_.find(it->first);
			if(it_agent == agentSubscribers_.end())
			{
				break;
			}
			it_agent->handler->send(boost::get<2>(dataElement));
		}
		it++;
	}
	sendBufferMap.clear();
}

//checks to see if the subscribed entity(agent) is alive
bool Broker::deadEntityCheck(sim_mob::JCommunicationSupport & info) {
	//some top notch optimizasion! to check if the agent is alive at all?
	info.cnt_1++;
	if (info.cnt_1 < 1000)
		return false;
	//you or your worker are probably dead already. you just don't know it
	if (!(info.getEntity().currWorker))
	{
		std::cout << "currWorker dead" << std::endl;
		return true;
	}
	//one more check to see if the entity is deleted
	const std::vector<sim_mob::Entity*> & managedEntities_ = info.getEntity().currWorker->getEntities();
	std::vector<sim_mob::Entity*>::const_iterator it = managedEntities_.begin();
	for (std::vector<sim_mob::Entity*>::const_iterator it =	managedEntities_.begin(); it != managedEntities_.end(); it++)
	{
		//agent is still being managed, so it is not dead
		if (*it == &(info.getEntity()))
			return false;
	}

	return true;
}

//iterate the entire subscription list looking for
//those who are not done with their update and check if they are dead.
//you better hope they are dead otherwise you have to hold the simulation
//tick waiting for them to finish
void Broker::refineSubscriptionList() {
	subscriberIterator it, it_end(subscriberList_.end());
	for(it = subscriberList_.begin(); it != it_end; it++)
	{
		const sim_mob::Agent * target = (*it).agent;
		//you or your worker are probably dead already. you just don't know it
		if (!target->currWorker)
			{
				unSubscribeEntity(target);
				continue;
			}
		const std::vector<sim_mob::Entity*> & managedEntities_ = (target->currWorker)->getEntities();
		std::vector<sim_mob::Entity*>::const_iterator  it_entity = std::find(managedEntities_.begin(), managedEntities_.end(), target);
		if(it_entity == managedEntities_.end())
		{
			unSubscribeEntity(target);
			continue;
		}
		else
		{
//			std::cout << std::dec << "_Agent [" << target << ":" << *it_entity << "] is still among " << (int)((target->currWorker)->getEntities().size()) << " entities of worker[" << target->currWorker << "]" << std::endl;
		}
	}
}
sim_mob::Broker sim_mob::Broker::instance(MtxStrat_Locked, 0);
Entity::UpdateStatus Broker::update(timeslice now)
{
	std::cout << "Broker tick:"<< now.frame() << " ================================================\n";
	if(now.frame() == 0)
		{
//			server_.start();
		}

	preparePerTickData(now);
	std::cout << "preparePerTickData Done" << std::endl;
	processIncomingData(now);
	std::cout << "processIncomingData Done" << std::endl;
	while(!allAgentUpdatesDone())
		{
			refineSubscriptionList();
			std::cout << "refineSubscriptionList Done" << std::endl;
		}
	std::cout << "allAgentUpdatesDone Done" << std::endl;
	processOutgoingData(now);
	std::cout << "processOutgoingData Done" << std::endl;

	std::cout <<"------------------------------------------------------\n";

	return UpdateStatus(UpdateStatus::RS_CONTINUE);

}

void Broker::unicast(const sim_mob::Agent * agent, std::string data)
{
	//find the connection handler
	agentIterator it = agentSubscribers_.find(agent);
	if(it == agentSubscribers_.end())
	{
		std::cout << "unicast Failed. Agent not Found" << std::endl;
	}
//	ConnectionHandler handler = it->handler;
	it->handler->send(data);
}

//todo write a more generic method ....later
void Broker::HandleMessage(MessageType type, MessageReceiver& sender,
                const Message& message)
{

}
}//namespace
