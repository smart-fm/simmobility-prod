//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/commsim/event/BaseCommsimEventArgs.hpp"

namespace sim_mob {

class Agent;

class LocationEventArgs: public BaseCommsimEventArgs {
public:
	LocationEventArgs(const sim_mob::Agent* agent);
	virtual ~LocationEventArgs();

	virtual std::string serialize() const;

private:
	const sim_mob::Agent* agent;

};

}

