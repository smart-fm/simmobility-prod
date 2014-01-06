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


sim_mob::comm::NS3_MSG_UNICAST::NS3_MSG_UNICAST(const sim_mob::comm::MsgData& data_): Message(data_)
{}

Handler* sim_mob::comm::NS3_MSG_UNICAST::newHandler()
{
	return new NS3_HDL_UNICAST();
}

//handler implementation
//you are going to handle something like this:
//{"MESSAGE_CAT":"APP","MESSAGE_TYPE":"UNICAST","UNICAST_DATA":"TVVMVElDQVNUIFN0cmluZyBmcm9tIGNsaWVudCAxMTQ=","RECEIVING_AGENT_ID":75,"SENDER":"0","SENDER_TYPE":"NS3_SIMULATOR"}

void sim_mob::comm::NS3_HDL_UNICAST::handle(sim_mob::comm::MsgPtr message_,Broker* broker)
{
}



