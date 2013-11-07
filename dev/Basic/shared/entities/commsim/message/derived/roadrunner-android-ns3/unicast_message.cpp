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
	return new sim_mob::roadrunner::UnicastHandler(true);
}


/*void sim_mob::rr_android_ns3::ANDROID_HDL_UNICAST::postProcess(sim_mob::Broker& broker, const sim_mob::Agent& destAgent, sim_mob::ClientHandler& destCliHandler, const std::string andrSensorId, const std::string& andrSensorType, sim_mob::comm::MsgData &data)
{
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
}*/


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



