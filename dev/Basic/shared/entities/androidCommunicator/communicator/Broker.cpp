#include "Broker.hpp"
#include "../../communicator/CommunicationSupport.hpp"
namespace sim_mob
{
void Broker::io_service_run(boost::asio::io_service & io_service_)
{
	server_.start();
	io_service_.run();
}

Broker::Broker(const MutexStrategy& mtxStrat, int id )
: Agent(mtxStrat, id),server_(io_service_,clientList) /*commImpl(&sendBuffer, &receiveBuffer,clientList)*/
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
		subscription_.sess = session_;
		//inserting to list
		agentList.at(&value.getEntity()) = subscription_;
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
	return UpdateStatus(UpdateStatus::RS_CONTINUE);

}
}//namespace
