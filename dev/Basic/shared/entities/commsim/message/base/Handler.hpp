//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/commsim/message/Types.hpp"

namespace sim_mob {

//Forward Declaration
class Broker;

///A message handler (no documentation provided).
class Handler {
public:
	virtual ~Handler() {}

	///Handle a given message.
	virtual void handle(sim_mob::comm::MsgPtr message_, sim_mob::Broker*) = 0;
};


}
