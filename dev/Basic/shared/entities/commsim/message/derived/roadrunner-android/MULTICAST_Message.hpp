//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MULTICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/base/Handler.hpp"

namespace sim_mob {
class Broker;

namespace roadrunner {

class MSG_MULTICAST : public sim_mob::comm::Message {
public:
	MSG_MULTICAST(sim_mob::comm::MsgData data_);
	sim_mob::Handler * newHandler();
};


//Handler for the above message
class HDL_MULTICAST : public sim_mob::Handler {
public:
	void handle(sim_mob::comm::MsgPtr message_, sim_mob::Broker* broker);
};

}}
