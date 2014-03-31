//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "BrokerBlocker.hpp"

namespace sim_mob {

class WaitForAgentRegistration: public sim_mob::BrokerBlocker {
public:
	WaitForAgentRegistration(sim_mob::Broker& broker, unsigned int minAgents = 1);
	virtual ~WaitForAgentRegistration();

	virtual bool calculateWaitStatus();

private:
	unsigned int minAgents;
	bool started; // has the broker previously satisfied the min_start criteria?

	//Note: I am disabling this for now; I expect the simulation should pause if the number of Agents is < minAgents.
	//      However, the simulation should really only pause for certain scenarios; this eventually needs to go in the config file.
	//unsigned int stop_threshold; //nof agents dropping 'below' this threshold will cause the broker to block
};

}

