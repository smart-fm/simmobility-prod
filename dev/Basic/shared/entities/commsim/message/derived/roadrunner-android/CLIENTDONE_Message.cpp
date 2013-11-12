//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * CLIENTDONE_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

//#include "CLIENTDONE_Handler.hpp"
#include "CLIENTDONE_Message.hpp"

using namespace sim_mob;

sim_mob::roadrunner::MSG_CLIENTDONE::MSG_CLIENTDONE(sim_mob::comm::MsgData& data_): Message(data_)
{}

Handler * sim_mob::roadrunner::MSG_CLIENTDONE::newHandler()
{
	return 0;
}



