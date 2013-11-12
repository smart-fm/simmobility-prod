//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * UNICAST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "UNICAST_Message.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"

using namespace sim_mob;

sim_mob::roadrunner::MSG_UNICAST::MSG_UNICAST(sim_mob::comm::MsgData& data_): Message(data_)
{}

Handler* sim_mob::roadrunner::MSG_UNICAST::newHandler()
{
	return new HDL_UNICAST();
}

//handler implementation
void sim_mob::roadrunner::HDL_UNICAST::handle(sim_mob::comm::MsgPtr message_,Broker* broker)
{
	//steps:
		/*
		 * 1- Find the destination agent and the corresponding client handler
		 * 2- fabricate a message for the destination(core data is taken from the original message)
		 * 3- insert messages into send buffer
		 */


	//	step-1: Find the target agent and the corresponding client handler
	sim_mob::comm::MsgData &data = message_->getData();
		sim_mob::msg_header msg_header_;
		if(!sim_mob::JsonParser::parseMessageHeader(data,msg_header_))
		{
			WarnOut( "HDL_UNICAST::handle: message header incomplete" << std::endl);
			return;
		}


		//	find the agent from the client
		//but,to get to the agent, find the client hander first

		//todo: you wanna check if the sender is valid, be my guest
		std::string receiver_id(data["RECEIVER"].asString()) ; //easy read
		std::string receiver_type(msg_header_.sender_type); //easy read

		ConfigParams::ClientType clientType;
		boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
		const ClientList::type & clients = broker->getClientList();
		if(!broker->getClientHandler(receiver_id,receiver_type,clnHandler))
		{
			WarnOut("HDL_UNICAST::handle failed" << std::endl);
			return;
		}

		//check the validity of the agent

		const sim_mob::Agent * destination_agent;
		if(!(destination_agent = clnHandler->agent))
		{
			WarnOut( "Invalid agent record. The Agent May Have completed its Operation and is Now out of simulation" << std::endl);
			return;
		}

		//you have a valid client handler now

		//step-2: fabricate a message for each(core data is taken from the original message)
			//actually, you dont need to modify any of
			//the original jsoncpp's Json::Value message.
			//just send it
		//so we go streight to next step
		//step-3: insert messages into send buffer
		broker->insertSendBuffer(clnHandler->cnnHandler,data);
}




