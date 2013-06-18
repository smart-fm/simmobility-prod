/*
 * UNICAST_Handler.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */
#include "UNICAST_Handler.hpp"
#include "entities/commsim/communicator/service/services.hpp"
#include "entities/commsim/communicator/serialization/Serialization.hpp"
#include "entities/commsim/communicator/service/services.hpp"
#include "entities/commsim/communicator/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/communicator/broker/Broker.hpp"
namespace sim_mob {
namespace roadrunner
{

void HDL_UNICAST::handle(msg_ptr message_,Broker* broker){
	//steps:
		/*
		 * 1- Find the destination agent and the corresponding client handler
		 * 2- fabricate a message for the destination(core data is taken from the original message)
		 * 3- insert messages into send buffer
		 */


	//	step-1: Find the target agent and the corresponding client handler
		Json::Value &data = message_->getData();
		sim_mob::msg_header msg_header_;
		if(sim_mob::JsonParser::parseMessageHeader(data,msg_header_))
		{
			Print() << "Sender : " << msg_header_.sender_id << "  sender_type : " << msg_header_.sender_type << std::endl;
		}
		else
		{
			Print() << "HDL_UNICAST::handle: message header incomplete" << std::endl;
			return;
		}


		if(
				data.isMember("RECEIVER_TYPE")
				&& data.isMember("RECEIVER")
				)
		{
			Print() <<
					"receiver : " << data["RECEIVER"] <<
					"  receiver_type : " << data["RECEIVER_TYPE"] <<std::endl;
		}

		//	find the agent from the client
		//but,to get to the agent, find the client hander first

		//todo: you wanna check if the sender is valid, be my guest
//		std::string sender_id(msg_header_.sender_id) ; //easy read
//		std::string sender_type(msg_header_.sender_type); //easy read
		std::string receiver_id(msg_header_.sender_id) ; //easy read
		std::string receiver_type(msg_header_.sender_type); //easy read

		ConfigParams::ClientType clientType;
		boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
		ClientList & clients = broker->getClientList();
		if(!broker->getClientHandler(receiver_id,receiver_type,clnHandler))
		{
			Print() << "HDL_UNICAST::handle failed" << std::endl;
			return;
		}
//		//use try catch to use map's .at() and search only once
//		try
//		{
//			clientType = ClientTypeMap.at(receiver_type);
//			std::map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > & inner = clients[clientType];
//			try
//			{
//				clnHandler = inner.at(receiver_id); //this is what we are looking for
//			}
//			catch(std::out_of_range e)
//			{
//				Print() << "Client " <<  receiver_id << " of type " <<  receiver_type << " not found" << std::endl;
//				return;
//			}
//		}
//		catch(std::out_of_range e)
//		{
//			Print() << "Client type" <<  receiver_type << " not found" << std::endl;
//			return;
//		}

		//now find the agent

		const sim_mob::Agent * destination_agent;
		if(!(destination_agent = clnHandler->agent))
		{
			Print() << "Invalid agent record. The Agent May Have completed its Operation and is Now out of simulation" << std::endl;
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

}/* namespace roadrunner */
} /* namespace sim_mob */



