//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MULTICAST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "MULTICAST_Message.hpp"

#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/AuraManager.hpp"

using namespace sim_mob;

sim_mob::roadrunner::MSG_MULTICAST::MSG_MULTICAST(sim_mob::comm::MsgData data_): Message(data_)
{}

Handler* sim_mob::roadrunner::MSG_MULTICAST::newHandler()
{
	return new HDL_MULTICAST();
}


//handler implementation
void sim_mob::roadrunner::HDL_MULTICAST::handle(sim_mob::comm::MsgPtr message_, Broker* broker){

//steps:
	/*
	 * 1- Find the target agent
	 * 2- Find its nearby agents
	 * 3- for each agent find the client handler
	 * 4- fabricate a message for each(core data is taken from the original message)
	 * 5- insert messages into send buffer
	 */

//	step-1: Find the target agent
	sim_mob::comm::MsgData &data = message_->getData();
	msg_header msg_header_;
	if(!sim_mob::JsonParser::parseMessageHeader(data,msg_header_))
	{
		WarnOut("HDL_MULTICAST::handle: message header incomplete" << std::endl);
		return;
	}

	//This check should always return true, but I'm adding it to hopefully merge the two classes.
	if(msg_header_.sender_type != "ANDROID_EMULATOR")
	{
		return;
	}

	//	find the agent from the client
	//but,to get to the agent, find the client hander first

	std::string sender_id(msg_header_.sender_id) ; //easy read
	std::string sender_type(msg_header_.sender_type); //easy read
	ConfigParams::ClientType clientType;
	boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
	if(!broker->getClientHandler(sender_id,sender_type,clnHandler))
	{
		WarnOut( "HDL_MULTICAST::handle failed" << std::endl);
		return;
	}

	//Check the handler's validity.
	//TODO: I ported this check from the other multicast_message class; need to make sure it's valid. ~Seth
	if(!clnHandler->isValid()) {
		Print() << "Invalid client handler record" << std::endl;
		return;
	}

	//now find the agent
	const sim_mob::Agent* original_agent = clnHandler->agent;
	if(!original_agent)
	{
		Print() << "Invalid agent record" << std::endl;
		return;
	}

	//step-2: get the agents around you
	std::vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(
		Point2D((original_agent->xPos - 3500), (original_agent->yPos - 3500)),
		Point2D((original_agent->xPos + 3500), (original_agent->yPos + 3500)),
		original_agent
	);

	//get the original agent out
	std::vector<const Agent*>::iterator it_find = std::find(nearby_agents.begin(), nearby_agents.end(), original_agent);
	if(it_find != nearby_agents.end()) {
		nearby_agents.erase(it_find);
	}

	//If there's no agents left, return.
	if(nearby_agents.size() == 0) {
		return;
	}

	//Now, Let's see which one of these agents are associated with clients
	ClientList::pair clientTypes;
	const ClientList::type& all_clients = broker->getClientList();
	BOOST_FOREACH(clientTypes , all_clients)
	{
		// only the android emulators
		//TODO: This should always return true; was copied over to hopefully merge the two classes together.
		if(clientTypes.first != ConfigParams::ANDROID_EMULATOR) {
			continue;
		}

		ClientList::IdPair clientIds;
		boost::unordered_map<std::string , boost::shared_ptr<sim_mob::ClientHandler> >& inner = clientTypes.second;
		BOOST_FOREACH(clientIds , inner)
		{
			boost::shared_ptr<sim_mob::ClientHandler> destClientHandlr  = clientIds.second;
			const sim_mob::Agent* agent = destClientHandlr->agent;
			if(std::find(nearby_agents.begin(), nearby_agents.end(), agent) == nearby_agents.end()) {
				continue;
			}

			//step-4: fabricate a message for each(core data is taken from the original message)
				//actually, you dont need to modify any of
				//the original jsoncpp's Json::Value message.
				//just send it
			//so we go streight to next step
			//step-5: insert messages into send buffer
			//NOTE: This part is different for ns-3 versus android-only.
			broker->insertSendBuffer(destClientHandlr->cnnHandler,data);
		}//inner loop : BOOST_FOREACH(clientIds , inner)
	}//outer loop : BOOST_FOREACH(clientTypes , clients)
}//handle()


