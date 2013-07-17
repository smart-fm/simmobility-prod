/*
 * UNICAST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "UNICAST_Message.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"

namespace sim_mob {
class Handler;

namespace rr_android_ns3
{
class HDL_UNICAST;
MSG_UNICAST::MSG_UNICAST(msg_data_t& data_): Message(data_)
{

}
Handler * MSG_UNICAST::newHandler()
{
	return new HDL_UNICAST();
}

//handler implementation

void HDL_UNICAST::handle(msg_ptr message_,Broker* broker){
	//steps:
		/*
		 * 1- Find the sending and destination node ids in ns3
		 * 2- fabricate a message for the destination(core data is taken from the original message)
		 * 3- insert the message into send buffer
		 */
	//	step-1: Find the target agent and the corresponding client handler
		Json::Value &data = message_->getData();
		sim_mob::msg_header msg_header_;
		if(!sim_mob::JsonParser::parseMessageHeader(data,msg_header_))
		{
			WarnOut( "HDL_UNICAST::handle: message header incomplete" << std::endl);
			return;
		}


		//	find the agent from the client
		//but,to get to the agent, find the client hander first

		//todo: you wanna check if the sender is valid, be my guest
		std::string android_sender_id(msg_header_.sender_id) ; //easy read
		std::string android_sender_type(msg_header_.sender_type); //easy read
		std::string android_receiver_id(data["RECEIVER"].asString()) ; //easy read
		std::string android_receiver_type(msg_header_.sender_type); //easy read, (same as sender)
		//find the ns3-equivalent names of the sender and receiver
		//note: the ns3-equivalent names happen to be same as the simmobility's agent ids

		boost::shared_ptr<sim_mob::ClientHandler> sending_clnHandler;
		boost::shared_ptr<sim_mob::ClientHandler> destination_clnHandler;
		//agent specification is specified in the client handler
		if(!broker->getClientHandler(android_sender_id,android_sender_type,sending_clnHandler))
		{
			WarnOut("HDL_UNICAST::sending_clnHandler not fount->handle failed" << std::endl);
			return;
		}

		if(!broker->getClientHandler(android_receiver_id,android_receiver_type,destination_clnHandler))
		{
			WarnOut("HDL_UNICAST::destination_clnHandler not fount->handle failed" << std::endl);
			return;
		}

		const sim_mob::Agent * sending_agent = sending_clnHandler->agent;
		const sim_mob::Agent * destination_agent = destination_clnHandler->agent;

//		//small check if clientHandler is ok!
//
//		if(!(clnHandler->isValid()))
//		{
//			WarnOut( "Invalid ns3 client record."<< std::endl);
//			return;
//		}

		//step-2: fabricate a message for each(core data is taken from the original message)
		data["RECEIVER"] = destination_agent->getId();
		data["SENDER"] = sending_agent->getId();

		//step-3: insert messages into send buffer
		boost::shared_ptr<sim_mob::ClientHandler> ns3_clnHandler;

		if(!broker->getClientHandler("0","NS3_SIMULATOR", ns3_clnHandler))
		{
			WarnOut("HDL_UNICAST::sending_clnHandler not fount->handle failed" << std::endl);
			return;
		}
		broker->insertSendBuffer(ns3_clnHandler->cnnHandler,data);
}

}/* namespace rr_android_ns3 */
} /* namespace sim_mob */



