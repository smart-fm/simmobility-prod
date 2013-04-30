#include "Broker.hpp"

#include <jsoncpp/json.h>

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
,subscriberList_(get<0>(getSubscriptionList()))
,agentSubscribers_(get<1>(getSubscriptionList()))
,clientSubscribers_(get<2>(getSubscriptionList()))
{
	//	io_service_thread = boost::thread(&Broker::io_service_run,this, io_service_);
	io_service_thread = boost::thread(&Broker::io_service_run,this, boost::ref(io_service_));
}

Broker::~Broker()
{
	io_service_thread.join();
}

//void Broker::assignClient(sim_mob::Entity *agent, std::pair<unsigned int,sim_mob::session_ptr> client)
//{
//	agentList[agent].second = client.second;
//}

std::vector<boost::shared_mutex*>  Broker::subscribeEntity(sim_mob::CommunicationSupport & value)
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
//		subscription_.session = session_;
		subscription_.handler.reset(new ConnectionHandler(session_,(MessageReceiver&)*this));
		boost::shared_ptr<ConnectionHandler> handler_(new ConnectionHandler(session_,(MessageReceiver&)*this));
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


void Broker::turnOnConnections()
{

}

void Broker::turnOnConnection(session_ptr session_)
{

}

sim_mob::Broker sim_mob::Broker::instance(MtxStrat_Locked, 0);
Entity::UpdateStatus Broker::update(timeslice now)
{
	//	commImpl.shortCircuit();
	std::cout << "communicator tick:"<< now.frame() << " ================================================\n";
////	printSubscriptionList(now);
//	boost::thread outGoing_thread(boost::bind(&sim_mob::NS3_Communicator::processOutgoingData,this,now));
//	boost::thread inComing_thread(boost::bind(&sim_mob::NS3_Communicator::processIncomingData,this,now));
//	outGoing_thread.join();
//	inComing_thread.join();
//	processOutgoingData(now);
//	processIncomingData(now);
//	reset();
	std::cout <<"------------------------------------------------------\n";

	while(ReadMessage());

	return UpdateStatus(UpdateStatus::RS_CONTINUE);

}

//todo write a more generic method ....later
void Broker::HandleMessage(MessageType type, MessageReceiver& sender,
                const Message& message)
{
	BrokerMessage* msg = MSG_CAST(BrokerMessage,message);
	switch(type)
	{
	case  ANNOUNCE:
	{
		//original data looks like this: {"messageType":"ANNOUNCE", "ANNOUNCE" : {"Sender":"clientIdxxx"}}
		//so the values of this method's argument list are: type = 1, sender = not important, message= "ANNOUNCE" : {"Sender":"clientIdxxx"}

		//first you need to find the nearby agents
		//extract the client id
		const Json::Value data = msg->root["ANNOUNCE"];
		unsigned int clientID = data["Sender"].asUInt();
		//get the mapping agent
		clientIterator it = clientSubscribers_.find(clientID);
		const Entity * entity = it->agent;
		const Agent * agent = dynamic_cast<const Agent *>(entity);
		//get agent's position
		int x,y;
		x = agent->xPos.get();
		y = agent->yPos.get();
		//get nearby agents

		vector<const Agent*> nearby_agents ;/*= AuraManager::instance().nearbyAgents(
				Point2D(x, y), *params.currLane, dis, distanceBehind);*/
		//seth:
		//ok, it is not possible to get the nearbyagents from agent (weird though)
		//let's fake it as you said.
		//todo: do the faking here....




//		sim_mob::Entity * receiver = (sim_mob::Entity *) (it->receiver);
		break;
	}
	case  KEY_REQUEST:
		break;
	case KEY_SEND:
		break;

	}
}
}//namespace
