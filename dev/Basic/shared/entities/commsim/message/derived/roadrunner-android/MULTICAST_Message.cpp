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
	return new sim_mob::roadrunner::MulticastHandler(false);
}

/*void sim_mob::roadrunner::HDL_MULTICAST::handleClient(const sim_mob::ClientHandler& clientHdlr, sim_mob::comm::MsgData& recipientsList, Broker& broker, sim_mob::comm::MsgData& data)
{
	broker.insertSendBuffer(clientHdlr.cnnHandler,data);
}

void sim_mob::roadrunner::HDL_MULTICAST::postPendingMessages(sim_mob::Broker& broker, const sim_mob::Agent& agent, const sim_mob::comm::MsgData& recipientsList, sim_mob::comm::MsgData& data)
{
}
*/

