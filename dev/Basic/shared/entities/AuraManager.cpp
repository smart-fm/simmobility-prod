//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AuraManager.hpp"

#include <algorithm>

#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/WayPoint.hpp"

#include "spatial_trees/TreeImpl.hpp"
#include "spatial_trees/rstar_tree/RStarAuraManager.hpp"
#include "spatial_trees/simtree/SimAuraManager.hpp"
#include "spatial_trees/rdu_tree/RDUAuraManager.hpp"

namespace sim_mob
{

AuraManager AuraManager::instance_;

////////////////////////////////////////////////////////////////////////////////////////////
// AuraManager
////////////////////////////////////////////////////////////////////////////////////////////

void AuraManager::init(AuraManagerImplementation implType)
{
	//Reset time tick.
	time_step = 0;

	if (implType == IMPL_RSTAR)
	{
		std::cout << "RSTAR" << std::endl;
		impl_ = new RStarAuraManager();
		impl_->init();
	}
	else if (implType == IMPL_SIMTREE)
	{
		std::cout << "SIMTREE" << std::endl;
		impl_ = new SimAuraManager();
		impl_->init();
	}
	else if (implType == IMPL_RDU)
	{
		std::cout << "RDU" << std::endl;
		impl_ = new RDUAuraManager();
		impl_->init();
	}
	else
	{
		throw std::runtime_error("Unknown tree type.");
	}
}

void AuraManager::destroy()
{
	delete impl_;
}

void AuraManager::update(const std::set<sim_mob::Entity *>& removedAgentPointers)
{
	if (impl_)
	{
		impl_->update(time_step, removedAgentPointers);
	}
	time_step++;
}

std::vector<Agent const *> AuraManager::agentsInRect(Point const &lowerLeft, Point const &upperRight, const sim_mob::Agent *refAgent) const
{
	std::vector<Agent const *> results;
	if (impl_)
	{
		results = impl_->agentsInRect(lowerLeft, upperRight, refAgent);
	}
	return results;
}


//The "refAgent" can be used to provide more information (i.e., for the faster bottom-up query).
std::vector<Agent const *> AuraManager::nearbyAgents(Point const &position, WayPoint const &wayPoint, double distanceInFront, double distanceBehind, 
													 const sim_mob::Agent *refAgent) const
{
	std::vector<Agent const *> results;
	if (impl_)
	{
		results = impl_->nearbyAgents(position, wayPoint, distanceInFront, distanceBehind, refAgent);
	}
	return results;
}

void AuraManager::registerNewAgent(Agent const *one_agent)
{
	if (impl_)
	{
		//We only register Person agents
		if (dynamic_cast<Person const*> (one_agent))
		{
			impl_->registerNewAgent(one_agent);
		}
	}
}

} // end of sim_mob


