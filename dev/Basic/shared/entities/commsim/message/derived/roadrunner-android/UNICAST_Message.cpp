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


void sim_mob::roadrunner::HDL_UNICAST::postProcess(sim_mob::Broker& broker, const sim_mob::Agent& destAgent, sim_mob::ClientHandler& destCliHandler, const std::string andrSensorId, const std::string& andrSensorType, sim_mob::comm::MsgData &data)
{
	broker.insertSendBuffer(destCliHandler.cnnHandler,data);
}



