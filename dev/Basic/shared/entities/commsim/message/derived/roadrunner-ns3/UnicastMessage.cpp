//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * UNICAST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "UnicastMessage.hpp"

#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"

using namespace sim_mob;


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



