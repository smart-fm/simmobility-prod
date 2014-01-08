//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/base/Handler.hpp"

namespace sim_mob {
namespace roadrunner {

//Handles a request to re-route.
class RerouteRequestHandler : public sim_mob::Handler {
public:
	virtual void handle(sim_mob::comm::MsgPtr message_, sim_mob::Broker* broker);
};

}}

