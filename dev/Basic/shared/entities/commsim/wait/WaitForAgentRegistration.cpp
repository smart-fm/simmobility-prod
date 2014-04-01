//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "WaitForAgentRegistration.hpp"
#include "entities/commsim/Broker.hpp"

using namespace sim_mob;

sim_mob::WaitForAgentRegistration::WaitForAgentRegistration() : BrokerBlocker(), minAgents(0)
{
}

sim_mob::WaitForAgentRegistration::~WaitForAgentRegistration()
{
}

void sim_mob::WaitForAgentRegistration::reset(unsigned int minAgents)
{
	this->minAgents = minAgents;
	passed = false;
}


bool WaitForAgentRegistration::calculateWaitStatus(BrokerBase& broker) const
{
	size_t numAg = broker.getRegisteredAgentsSize();
	return numAg>=minAgents;
}

