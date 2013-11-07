//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * UNICAST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "unicast_message.hpp"

#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"

using namespace sim_mob;


sim_mob::rr_android_ns3::ANDROID_MSG_UNICAST::ANDROID_MSG_UNICAST(sim_mob::comm::MsgData& data_): Message(data_)
{}

Handler* sim_mob::rr_android_ns3::ANDROID_MSG_UNICAST::newHandler()
{
	return new ANDROID_HDL_UNICAST();
}


//handler implementation
void sim_mob::rr_android_ns3::ANDROID_HDL_UNICAST::handle(sim_mob::comm::MsgPtr message_,Broker* broker)
{
	//steps:
		/*
		 * 1- Find the sending and destination node ids in ns3
		 * 2- fabricate a message for the destination(core data is taken from the original message)
		 * 3- insert the message into send buffer
		 */
	//	step-1: Find the target agent and the corresponding client handler
	sim_mob::comm::MsgData &data = message_->getData();
		sim_mob::msg_header msg_header_;
		if(!sim_mob::JsonParser::parseMessageHeader(data,msg_header_))
		{
			WarnOut( "ANDROID_HDL_UNICAST::handle: message header incomplete" << std::endl);
			return;
		}


		//	find the agent from the client
		//but,to get to the agent, find the client hander first

		//These variables are only used by ns-3
		std::string android_sender_id(msg_header_.sender_id) ; //easy read
		std::string android_sender_type(msg_header_.sender_type); //easy read

		//todo: you wanna check if the sender is valid, be my guest
		std::string receiver_id(data["RECEIVER"].asString()) ; //easy read
		std::string receiver_type(msg_header_.sender_type); //easy read, (same as sender)
		//find the ns3-equivalent names of the sender and receiver
		//note: the ns3-equivalent names happen to be same as the simmobility's agent ids

		boost::shared_ptr<sim_mob::ClientHandler> destClnHandler;
		if(!broker->getClientHandler(receiver_id,receiver_type,destClnHandler))
		{
			WarnOut("ANDROID_HDL_UNICAST::destination_clnHandler not fount->handle failed" << std::endl);
			return;
		}

		const sim_mob::Agent * destination_agent = destClnHandler->agent;

		//NOTE: I am assuming it is ok to check this here. ~Seth.
		if(!destination_agent) {
			WarnOut( "Invalid agent record. The Agent May Have completed its Operation and is Now out of simulation" << std::endl);
			return;
		}

//		//small check if clientHandler is ok!
//
//		if(!(clnHandler->isValid()))
//		{
//			WarnOut( "Invalid ns3 client record."<< std::endl);
//			return;
//		}


		//NOTE: The following is different for ns-3 versus android-only
		boost::shared_ptr<sim_mob::ClientHandler> senderClnHandler;
		if(!broker->getClientHandler(android_sender_id,android_sender_type,senderClnHandler))
		{
			WarnOut("ANDROID_HDL_UNICAST::sending_clnHandler not fount->handle failed" << std::endl);
			return;
		}
		const sim_mob::Agent * sending_agent = senderClnHandler->agent;
		//step-2: fabricate a message for each(core data is taken from the original message)
		data["RECEIVER"] = destination_agent->getId();
		data["SENDER"] = sending_agent->getId();
		//step-3: insert messages into send buffer
		boost::shared_ptr<sim_mob::ClientHandler> ns3_clnHandler;
		if(!broker->getClientHandler("0","NS3_SIMULATOR", ns3_clnHandler)) {
			WarnOut("ANDROID_HDL_UNICAST::sending_clnHandler not fount->handle failed" << std::endl);
			return;
		}
		broker->insertSendBuffer(ns3_clnHandler->cnnHandler,data);
}


sim_mob::rr_android_ns3::NS3_MSG_UNICAST::NS3_MSG_UNICAST(sim_mob::comm::MsgData& data_): Message(data_)
{}

Handler* sim_mob::rr_android_ns3::NS3_MSG_UNICAST::newHandler()
{
	return new NS3_HDL_UNICAST();
}

//handler implementation
void sim_mob::rr_android_ns3::NS3_HDL_UNICAST::handle(sim_mob::comm::MsgPtr message_,Broker* broker)
{
}



