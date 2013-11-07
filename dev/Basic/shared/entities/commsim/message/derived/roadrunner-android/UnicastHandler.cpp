//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * UNICAST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "UnicastHandler.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"

using namespace sim_mob;

sim_mob::roadrunner::UnicastHandler::UnicastHandler(bool useNs3) : useNs3(useNs3)
{
}


//handler implementation
void sim_mob::roadrunner::UnicastHandler::handle(sim_mob::comm::MsgPtr message_,Broker* broker)
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

		//These variables are only used by ns-3
		std::string android_sender_id(msg_header_.sender_id) ; //easy read
		std::string android_sender_type(msg_header_.sender_type); //easy read

		//todo: you wanna check if the sender is valid, be my guest
		std::string receiver_id(data["RECEIVER"].asString()) ; //easy read
		std::string receiver_type(msg_header_.sender_type); //easy read

		boost::shared_ptr<sim_mob::ClientHandler> destClnHandler;
		if(!broker->getClientHandler(receiver_id,receiver_type,destClnHandler))
		{
			WarnOut("HDL_UNICAST::handle failed" << std::endl);
			return;
		}

		//check the validity of the agent

		const sim_mob::Agent * destination_agent = destClnHandler->agent;
		if(!destination_agent) {
			WarnOut( "Invalid agent record. The Agent May Have completed its Operation and is Now out of simulation" << std::endl);
			return;
		}


		//NOTE: The following is different for ns-3 versus android-only
		postProcess(*broker, *destination_agent, *destClnHandler, android_sender_id, android_sender_type, data);
}


void sim_mob::roadrunner::UnicastHandler::postProcess(sim_mob::Broker& broker, const sim_mob::Agent& destAgent, sim_mob::ClientHandler& destCliHandler, const std::string andrSensorId, const std::string& andrSensorType, sim_mob::comm::MsgData &data)
{
	if (useNs3) {
		boost::shared_ptr<sim_mob::ClientHandler> senderClnHandler;
		if(!broker.getClientHandler(andrSensorId, andrSensorType, senderClnHandler)) {
			WarnOut("ANDROID_HDL_UNICAST::sending_clnHandler not fount->handle failed" << std::endl);
			return;
		}
		const sim_mob::Agent * sendAgent = senderClnHandler->agent;
		//step-2: fabricate a message for each(core data is taken from the original message)
		data["RECEIVER"] = destAgent.getId();
		data["SENDER"] = sendAgent->getId();
		//step-3: insert messages into send buffer
		boost::shared_ptr<sim_mob::ClientHandler> ns3_clnHandler;
		if(!broker.getClientHandler("0","NS3_SIMULATOR", ns3_clnHandler)) {
			WarnOut("ANDROID_HDL_UNICAST::sending_clnHandler not fount->handle failed" << std::endl);
			return;
		}
		broker.insertSendBuffer(ns3_clnHandler->cnnHandler,data);
	} else {
		broker.insertSendBuffer(destCliHandler.cnnHandler,data);
	}
}



