/*
 * ANNOUNCE_Handler.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */
#include "ANNOUNCE_Handler.hpp"
#include "entities/commsim/communicator/service/services.hpp"
#include "entities/commsim/communicator/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/communicator/broker/Broker.hpp"
#include "entities/AuraManager.hpp"

namespace sim_mob {
namespace roadrunner
{

void HDL_ANNOUNCE::handle(msg_ptr message_,Broker* broker){

//steps:
	/*
	 * 1- Find the target agent
	 * 2- Find its nearby agents
	 * 3- for each agent find the client handler
	 * 4- fabricate a message for each(core data is taken from the original message)
	 * 5- insert messages into send buffer
	 */

//	step-1: Find the target agent
	Json::Value &data = message_->getData();

	Print() << "HDL_ANNOUNCE::handle is handling a mesage" << std::endl;
	if(data.isMember("SENDER_TYPE") && data.isMember("SENDER"))
	{
		Print() << "Sender : " << data["SENDER"] << "  sender_type : " << data["SENDER_TYPE"] << std::endl;
	}

	//	find the agent from the client
	//but,to get to the agent, find the client hander first

	std::string sender_id(data["SENDER"].asString()) ; //easy read
	std::string sender_type = data["SENDER_TYPE"].asString(); //easy read
	ConfigParams::ClientType clientType;
	boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
	ClientList & clients = broker->getClientList();
	//use try catch to use map's .at() and search only once
	try
	{
		Print() << "HDL_ANNOUNCE::handle 1" << std::endl;
		clientType = ClientTypeMap.at(sender_type);
		std::map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > & inner = clients[clientType];
		Print() << "HDL_ANNOUNCE::handle 2" << std::endl;
		try
		{
			clnHandler = inner.at(sender_id); //this is what we are looking for
			Print() << "HDL_ANNOUNCE::handle 3" << std::endl;
		}
		catch(std::out_of_range e)
		{
			Print() << "Client " <<  sender_id << " of type " <<  sender_type << " not found" << std::endl;
			Print() << "HDL_ANNOUNCE::handle 4" << std::endl;
			return;
		}

		Print() << "HDL_ANNOUNCE::handle 5" << std::endl;
	}
	catch(std::out_of_range e)
	{

		Print() << "HDL_ANNOUNCE::handle 6" << std::endl;
		Print() << "Client type" <<  sender_type << " not found" << std::endl;
		return;
	}

	//now find the agent

	const sim_mob::Agent * original_agent;
	if(!(original_agent = clnHandler->agent))
	{
		Print() << "Invalid agent record" << std::endl;
		return;
	}

	Print() << "HDL_ANNOUNCE::handle 7" << std::endl;
	//step-2: get the agents around you
	std::vector<const Agent*> nearby_agents_1 = AuraManager::instance().agentsInRect(

				Point2D(

						(original_agent->xPos - 3500),
						(original_agent->yPos - 3500)
						)

						,

				Point2D(
						(original_agent->xPos + 3500),
						(original_agent->yPos + 3500)
						)

						);
	Print() << "HDL_ANNOUNCE::handle 8" << std::endl;
	//if no agent found or only one found and that is the original_agent
	if((nearby_agents_1.size() == 0) || ( (nearby_agents_1.size() == 1)&&(nearby_agents_1[0] == original_agent)) )
	{
		Print() << "No agents around " << original_agent << std::endl;
		return;
	}

	Print() << "HDL_ANNOUNCE::handle 9" << std::endl;
	//get the original agent out
	std::vector<const Agent*>::iterator it_find = std::find(nearby_agents_1.begin(), nearby_agents_1.end(), original_agent);
	if(it_find != nearby_agents_1.end())
	{
		nearby_agents_1.erase(it_find);
	}
//	Print() << "Found " << nearby_agents_1.size() << " agents around " << original_agent << " Let's c which one can communicate"<< std::endl;
	//Now, Let's c which one of these agents are associated with clients
	std::vector<const Agent*> nearby_agents_2;
	std::pair<unsigned int, std::map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > > clientTypes;
	BOOST_FOREACH(clientTypes , clients)
	{
		Print() << "HDL_ANNOUNCE::handle 10" << std::endl;
		std::pair<std::string , boost::shared_ptr<sim_mob::ClientHandler> > clientIds;
		std::map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > &inner = clientTypes.second;
		//step-3: for each agent find the client handler
		BOOST_FOREACH(clientIds , inner)
		{
			Print() << "HDL_ANNOUNCE::handle 11" << std::endl;
			boost::shared_ptr<sim_mob::ClientHandler> clnHander  = clientIds.second;
			const sim_mob::Agent * agent = clnHander->agent;
			if(std::find(nearby_agents_1.begin(), nearby_agents_1.end(), agent) == nearby_agents_1.end())
			{
				continue;
			}
			Print() << "HDL_ANNOUNCE::handle 12" << std::endl;
			Print() << "Oh, found a communicating agent around" << std::endl;
			//step-4: fabricate a message for each(core data is taken from the original message)
				//actually, you dont need to modify any of
				//the original jsoncpp's Json::Value message.
				//just send it
			//so we go streight to next step
			//step-5: insert messages into send buffer
			broker->insertSendBuffer(clnHander->cnnHandler,data);
			Print() << "HDL_ANNOUNCE::handle 13" << std::endl;
		}//inner loop : BOOST_FOREACH(clientIds , inner)
	}//outer loop : BOOST_FOREACH(clientTypes , clients)
}//handle()

}/* namespace roadrunner */
} /* namespace sim_mob */



