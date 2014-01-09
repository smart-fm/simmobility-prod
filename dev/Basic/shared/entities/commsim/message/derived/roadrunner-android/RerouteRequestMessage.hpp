//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/derived/roadrunner-android/RerouteRequestHandler.hpp"

namespace sim_mob {
namespace roadrunner {

class RerouteRequestMessage : public sim_mob::comm::Message {
public:
	RerouteRequestMessage(const sim_mob::comm::MsgData& data_) : Message(data_)
	{}

	sim_mob::Handler* newHandler() {
		return new sim_mob::roadrunner::RerouteRequestHandler();
	}
};

}}

