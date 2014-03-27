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
#include "entities/commsim/message/derived/gen-android/MulticastHandler.hpp"

namespace sim_mob {
namespace comm {

/*class MulticastMessage : public sim_mob::comm::Message {
public:
	MulticastMessage(const sim_mob::comm::MsgData& data_, bool useNs3) : Message(data_), useNs3(useNs3)
	{}

	sim_mob::Handler* newHandler() {
		return new sim_mob::comm::MulticastHandler(useNs3);
	}

private:
	bool useNs3;
};*/


}}
