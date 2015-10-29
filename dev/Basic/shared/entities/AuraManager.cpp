//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AuraManager.hpp"

#include <algorithm>

#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Point.hpp"

#include "spatial_trees/TreeImpl.hpp"
#include "spatial_trees/rstar_tree/RStarAuraManager.hpp"
#include "spatial_trees/rdu_tree/RDUAuraManager.hpp"

namespace sim_mob
{
/* static */ AuraManager AuraManager::instance_;

////////////////////////////////////////////////////////////////////////////////////////////
// AuraManager
////////////////////////////////////////////////////////////////////////////////////////////

void
AuraManager::init(AuraManagerImplementation implType)
{
    //Reset time tick.
    time_step = 0;

	if (implType == IMPL_RSTAR) {
		std::cout << "RSTAR" << std::endl;
		impl_ = new RStarAuraManager();
		impl_->init();
	} else if (implType == IMPL_RDU) {
		std::cout << "RDU" << std::endl;
		impl_ = new RDUAuraManager();
		impl_->init();
	} else {
		throw std::runtime_error("Unknown tree type.");
	}
}

void AuraManager::destory()
{
	delete impl_;
}

/* virtual */ void
AuraManager::update(const std::set<sim_mob::Entity*>& removedAgentPointers)
{

	if (impl_) {
		impl_->update(time_step, removedAgentPointers);
	}
	time_step++;

}

std::vector<Agent const *>
AuraManager::agentsInRect(Point const & lowerLeft, Point const & upperRight, const sim_mob::Agent* refAgent)
const
{
	std::vector<Agent const *> results;
	if (impl_) {
		results = impl_->agentsInRect(lowerLeft, upperRight, refAgent);
	}
	return results;
}


//The "refAgent" can be used to provide more information (i.e., for the faster bottom-up query).
std::vector<Agent const *>
AuraManager::nearbyAgents(Point const & position, Lane const & lane,
                          centimeter_t distanceInFront, centimeter_t distanceBehind, const sim_mob::Agent* refAgent)
const
{
	std::vector<Agent const *> results;
	if (impl_) {
		results = impl_->nearbyAgents(position, lane, distanceInFront, distanceBehind, refAgent);
	}
	return results;
}

void AuraManager::registerNewAgent(Agent const* one_agent)
{
	//if ((local_implType == IMPL_SIMTREE) && impl_) {
	if (impl_) {
		//We only register Person agents (TODO: why?)
		if (dynamic_cast<Person const*>(one_agent)) {
			impl_->registerNewAgent(one_agent);
		}
	}
}

} // end of sim_mob


