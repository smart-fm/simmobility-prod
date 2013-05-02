#include "Broker.hpp"
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <jsoncpp/json.h>
#include "Message/RoadRunnerHandler.hpp"
#include "entities/AuraManager.hpp"

namespace sim_mob
{
void Broker::io_service_run(boost::asio::io_service & io_service_)
{
	server_.start();
	io_service_.run();
}
subscriptionC &Broker::getSubscriptionList() { return subscriptionList;}
Broker::Broker(const MutexStrategy& mtxStrat, int id )
: Agent(mtxStrat, id),server_(io_service_,clientList) /*commImpl(&sendBuffer, &receiveBuffer,clientList)*/
,Broker_Mutex(new boost::shared_mutex)
,Broker_Mutex_Send(new boost::shared_mutex)
,Broker_Mutex_Receive(new boost::shared_mutex)
,sendBuffer(Broker_Mutex_Send)
,receiveBuffer(Broker_Mutex_Receive)
,subscriberList_(get<0>(getSubscriptionList()))
,agentSubscribers_(get<1>(getSubscriptionList()))
,clientSubscribers_(get<2>(getSubscriptionList()))
{
	 MessageMap = boost::assign::map_list_of("ANNOUNCE", ANNOUNCE)("KEY_REQUEST", KEY_REQUEST)("KEY_SEND",KEY_SEND);
	io_service_thread = boost::thread(&Broker::io_service_run,this, boost::ref(io_service_));
}

Broker::~Broker()
{
	io_service_thread.join();
}

//called when a message is received in a connectionHandler
//so far there is no need to specify wich connection it is
//coz it is currently assumed that the string contains all the information
void Broker::handleReceiveMessage(std::string str){
	receiveBuffer.add(sim_mob::BufferContainer::makeDataElement(str));
}

std::vector< boost::shared_ptr<boost::shared_mutex> >  Broker::subscribeEntity(sim_mob::CommunicationSupport & value)
{
	subscription subscription_(&value);

	if(clientList.size())
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
		subscription_.handler.reset(new sim_mob::ConnectionHandler(session_,*this, &Broker::handleReceiveMessage, clientID_, (unsigned long)const_cast<sim_mob::Agent*>(&value.getEntity())));
		//inserting to list
		subscriberList_.insert(subscriberList_.begin(),subscription_);
		//start send/receive functionality right here. It seems there is no need of a thread
		(subscription_.handler)->start();
	}
	else
	{
		//no client(android emulator for now) available, putting into the waiting list
		agentWaitingList.at(&value.getEntity()) = subscription_;
	}

	return mutex_collection;
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

void Broker::processOutgoingData(timeslice now)
{
//	now send what you have to send:

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
sim_mob::Broker sim_mob::Broker::instance(MtxStrat_Locked, 0);
Entity::UpdateStatus Broker::update(timeslice now)
{
	std::cout << "communicator tick:"<< now.frame() << " ================================================\n";
////	printSubscriptionList(now);
//	boost::thread outGoing_thread(boost::bind(&sim_mob::NS3_Communicator::processOutgoingData,this,now));
//	boost::thread inComing_thread(boost::bind(&sim_mob::NS3_Communicator::processIncomingData,this,now));
//	outGoing_thread.join();
//	inComing_thread.join();
//	processOutgoingData(now);
//	processIncomingData(now);
//	reset();

	preparePerTickData(now);
	processIncomingData(now);
	processOutgoingData(now);

	std::cout <<"------------------------------------------------------\n";

	while(ReadMessage());

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
//	/*
//	 * Important Note.
//	 * I just have a doubt if full functionalities of
//	 * a common message handling architecture
//	 * can be exploited in our time-based simulator.
//	 * messages in our simulator are processed within
//	 * a 'tick' time window, whereas this callback method
//	 * "HandleMessage" doesn't respect this framework and is invoked
//	 * whenever there is a message in the message queue.
//	 * So, until i figure out how to make best use of message handling within
//	 * our framework, any messages passed to this callback will be reposted to
//	 * our own buffer to be processed in each tick.
//	 */
//
//	BrokerMessage* msg = MSG_CAST(BrokerMessage,message);
//	receiveBuffer.add(boost::make_tuple(type, msg->str, msg->root));
//	return;
//	//todo move this part to your handler
//	switch(type)
//	{
//	case  ANNOUNCE:
//	{
//		//original data looks like this: {"messageType":"ANNOUNCE", "ANNOUNCE" : {"Sender":"clientIdxxx"}}
//		//so the values of this method's argument list are: type = 1, sender = not important, message= "ANNOUNCE" : {"Sender":"clientIdxxx"}
//
//		//first you need to find the nearby agents
//		//extract the client id
//		const Json::Value data = msg->root["ANNOUNCE"];
//		unsigned int clientID = data["Sender"].asUInt();
//		//get the mapping agent
//		clientIterator it = clientSubscribers_.find(clientID);
//		const Entity * entity = it->agent;
//		const Agent * agent = dynamic_cast<const Agent *>(entity);
//		//get agent's position
//		int x,y;
//		x = agent->xPos.get();
//		y = agent->yPos.get();
//		//get nearby agents
//
//		std::vector<const Agent*> nearby_agents ;/*= AuraManager::instance().nearbyAgents(
//
//				Point2D(x, y), *params.currLane, dis, distanceBehind);*/
//		//seth:
//		//ok, it is not possible to get the nearbyagents from agent (weird though)
//		//let's fake it as you said.
//		//todo: do the faking here....
//
//
//
//
////		sim_mob::Entity * receiver = (sim_mob::Entity *) (it->receiver);
//		break;
//	}
//	case  KEY_REQUEST:
//		break;
//	case KEY_SEND:
//		break;
//
//	}
}
}//namespace
