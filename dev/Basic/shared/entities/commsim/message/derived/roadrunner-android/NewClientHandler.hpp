//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <stdexcept>

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/base/Handler.hpp"

namespace sim_mob {
namespace roadrunner {

//Handles a request for a new client to be added.
class NewClientHandler : public sim_mob::Handler {
public:
	virtual void handle(sim_mob::comm::MsgPtr message_, sim_mob::Broker* broker) {
		throw std::runtime_error("NEW_CLIENT not yet supported.");
	}
};

}}

