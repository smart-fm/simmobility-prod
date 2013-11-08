//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * CLIENTDONE_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"

namespace sim_mob {
namespace rr_android_ns3 {

class MSG_CLIENTDONE : public sim_mob::comm::Message {
public:
	MSG_CLIENTDONE(sim_mob::comm::MsgData& data_);
	Handler * newHandler();
};

}}
