/*
 * MULTICAST_Handler.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */
#include "MULTICAST_Handler.hpp"
#include "entities/commsim/communicator/service/services.hpp"
#include "entities/commsim/communicator/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/communicator/broker/Broker.hpp"
#include "entities/AuraManager.hpp"

namespace sim_mob {
namespace roadrunner
{

void HDL_MULTICAST::handle(msg_ptr message_,Broker* broker){

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
	msg_header msg_header_;
	if(sim_mob::JsonParser::parseMessageHeader(data,msg_header_))
	{
		Print() << "Sender : " << msg_header_.sender_id << "  sender_type : " << msg_header_.sender_type << std::endl;
	}
	else
	{
		Print() << "HDL_MULTICAST::handle: message header incomplete" << std::endl;
		return;
	}

	//	find the agent from the client
	//but,to get to the agent, find the client hander first

	std::string sender_id(msg_header_.sender_id) ; //easy read
	std::string sender_type(msg_header_.sender_type); //easy read
	ConfigParams::ClientType clientType;
	boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
	ClientList & clients = broker->getClientList();
	if(!broker->getClientHandler(sender_id,sender_type,clnHandler))
	{
		Print() << "HDL_MULTICAST::handle failed" << std::endl;
		return;
	}
//	//use try catch to use map's .at() and search only once
//	try
//	{
//		clientType = ClientTypeMap.at(sender_type);
//		std::map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > & inner = clients[clientType];
//		try
//		{
//			clnHandler = inner.at(sender_id); //this is what we are looking for
//		}
//		catch(std::out_of_range e)
//		{
//			Print() << "Client " <<  sender_id << " of type " <<  sender_type << " not found" << std::endl;
//			return;
//		}
//
//	}
//	catch(std::out_of_range e)
//	{
//
//		Print() << "Client type" <<  sender_type << " not found" << std::endl;
//		return;
//	}

	//now find the agent

	const sim_mob::Agent * original_agent;
	if(!(original_agent = clnHandler->agent))
	{
		Print() << "Invalid agent record" << std::endl;
		return;
	}

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
	//if no agent found or only one found and that is the original_agent
	if((nearby_agents_1.size() == 0) || ( (nearby_agents_1.size() == 1)&&(nearby_agents_1[0] == original_agent)) )
	{
		Print() << "No agents around " << original_agent << std::endl;
		return;
	}

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
		std::pair<std::string , boost::shared_ptr<sim_mob::ClientHandler> > clientIds;
		std::map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > &inner = clientTypes.second;
		//step-3: for each agent find the client handler
		BOOST_FOREACH(clientIds , inner)
		{
			boost::shared_ptr<sim_mob::ClientHandler> clnHander  = clientIds.second;
			const sim_mob::Agent * agent = clnHander->agent;
			if(std::find(nearby_agents_1.begin(), nearby_agents_1.end(), agent) == nearby_agents_1.end())
			{
				continue;
			}
			Print() << "Oh, found a communicating agent around" << std::endl;
			//step-4: fabricate a message for each(core data is taken from the original message)
				//actually, you dont need to modify any of
				//the original jsoncpp's Json::Value message.
				//just send it
			//so we go streight to next step
			//step-5: insert messages into send buffer
			broker->insertSendBuffer(clnHander->cnnHandler,data);
		}//inner loop : BOOST_FOREACH(clientIds , inner)
	}//outer loop : BOOST_FOREACH(clientTypes , clients)
}//handle()

}/* namespace roadrunner */
} /* namespace sim_mob */



