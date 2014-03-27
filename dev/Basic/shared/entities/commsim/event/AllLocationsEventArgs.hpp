//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "entities/commsim/event/BaseCommsimEventArgs.hpp"

namespace sim_mob {

class AgentsList;

class AllLocationsEventArgs: public BaseCommsimEventArgs {
public:
	AllLocationsEventArgs(const sim_mob::AgentsList& registeredAgents);
	virtual ~AllLocationsEventArgs();

	virtual std::string serialize() const;

private:
	const sim_mob::AgentsList& registeredAgents;
};

}

