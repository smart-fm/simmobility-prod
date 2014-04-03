//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AllLocationsEventArgs.hpp"

#include "entities/commsim/serialization/CommsimSerializer.hpp"
#include "entities/commsim/broker/Broker.hpp"
#include "entities/Agent.hpp"

using namespace sim_mob;

sim_mob::AllLocationsEventArgs::AllLocationsEventArgs(const std::map<const Agent*, AgentInfo>& registeredAgents) : registeredAgents(registeredAgents)
{
}

sim_mob::AllLocationsEventArgs::~AllLocationsEventArgs()
{
}

std::string sim_mob::AllLocationsEventArgs::serialize() const
{
	std::map<unsigned int, DPoint> allLocs;
	for (std::map<const Agent*, AgentInfo>::const_iterator it=registeredAgents.begin(); it!=registeredAgents.end(); it++) {
		allLocs[it->first->getId()] = DPoint(it->first->xPos.get(), it->first->yPos.get());
	}

	return CommsimSerializer::makeAllLocations(allLocs);
}


