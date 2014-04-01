//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AllLocationsEventArgs.hpp"

#include "entities/commsim/serialization/CommsimSerializer.hpp"
#include "entities/commsim/broker/Broker-util.hpp"
#include "entities/Agent.hpp"

using namespace sim_mob;

sim_mob::AllLocationsEventArgs::AllLocationsEventArgs(const AgentsList& registeredAgents) : registeredAgents(registeredAgents)
{
}

sim_mob::AllLocationsEventArgs::~AllLocationsEventArgs()
{
}

std::string sim_mob::AllLocationsEventArgs::serialize() const
{
	std::map<unsigned int, DPoint> allLocs;

	//NOTE: I am fairly sure that this is only ever called in a thread-safe context. ~Seth
	const AgentsList::type& aList = registeredAgents.getAgents();
	for (sim_mob::AgentsList::type::const_iterator it=aList.begin(); it!=aList.end(); it++) {
		allLocs[it->first->getId()] = DPoint(it->first->xPos.get(), it->first->yPos.get());
	}

	return CommsimSerializer::makeAllLocations(allLocs);
}


