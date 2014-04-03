//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include <map>

#include "entities/commsim/event/BaseCommsimEventArgs.hpp"

namespace sim_mob {

class Agent;
struct AgentInfo;

class AllLocationsEventArgs: public BaseCommsimEventArgs {
public:
	AllLocationsEventArgs(const std::map<const Agent*, AgentInfo>& registeredAgents);
	virtual ~AllLocationsEventArgs();

	virtual std::string serialize() const;

private:
	const std::map<const Agent*, AgentInfo>& registeredAgents;
};

}

