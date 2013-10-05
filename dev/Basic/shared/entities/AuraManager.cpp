//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AuraManager.hpp"

#include <algorithm>

#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Point2D.hpp"

#include "spatial_trees/TreeImpl.hpp"
#include "spatial_trees/rstar_tree/RStarAuraManager.hpp"
#include "spatial_trees/simtree/SimAuraManager.hpp"
#include "spatial_trees/rdu_tree/RDUAuraManager.hpp"

namespace sim_mob
{
/* static */ AuraManager AuraManager::instance_;
///* static */ AuraManager AuraManager::instance2_;

/** \cond ignoreAuraManagerInnards -- Start of block to be ignored by doxygen.  */

////////////////////////////////////////////////////////////////////////////////////////////
// AuraManager::Stats
////////////////////////////////////////////////////////////////////////////////////////////

/*struct AuraManager::Stats : private boost::noncopyable
{
    void
    printStatistics() const;
};

void
AuraManager::Stats::printStatistics() const
{
    std::cout << "AuraManager::Stats not implemented yet" << std::endl;
    //implementing for mid-term specific stats


}
*/
/** \endcond ignoreAuraManagerInnards -- End of block to be ignored by doxygen.  */

////////////////////////////////////////////////////////////////////////////////////////////
// AuraManager
////////////////////////////////////////////////////////////////////////////////////////////

void
AuraManager::init(AuraManagerImplementation implType)
{
/*    if (false) {
        stats_ = new Stats;
    }*/

    //this->local_implType = implType;

    //Reset time tick.
    time_step = 0;

	if (implType == IMPL_RSTAR) {
		std::cout << "RSTAR" << std::endl;
		impl_ = new RStarAuraManager();
		impl_->init();
	} else if (implType == IMPL_SIMTREE) {
		std::cout << "SIMTREE" << std::endl;
		impl_ = new SimAuraManager();
		impl_->init();
	} else if (implType == IMPL_RDU) {
		std::cout << "RDU" << std::endl;
		impl_ = new RDUAuraManager();
		impl_->init();
	} else {
		throw std::runtime_error("Unknown tree type.");
	}
}

/* virtual */ void
AuraManager::update(const std::set<sim_mob::Agent*>& removedAgentPointers)
{

	if (impl_) {
		impl_->update(time_step, removedAgentPointers);
	}

	time_step++;

}

std::vector<Agent const *>
AuraManager::agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight, const sim_mob::Agent* refAgent)
const
{
	std::vector<Agent const *> results;
	if (impl_) {
		results = impl_->agentsInRect(lowerLeft, upperRight, refAgent);
	}
	return results;
}

/*std::vector<Agent const *>
AuraManager::advanced_agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight, TreeItem* item) const
{
	if(local_implType != IMPL_SIMTREE)
	{
		return agentsInRect(lowerLeft, upperRight);
	}

	std::vector<Agent const *> results;
	if (impl_) {
		results = impl_->advanced_agentsInRect(lowerLeft, upperRight, item);
	}

//	static long sum_count = 0;
//	sum_count += results.size();
//	std::cout << "advanced_agentsInRect:" << results.size() << ",sum_count:" << sum_count << std::endl;

	return results;
}*/

//The "refAgent" can be used to provide more information (i.e., for the faster bottom-up query).
std::vector<Agent const *>
AuraManager::nearbyAgents(Point2D const & position, Lane const & lane,
                          centimeter_t distanceInFront, centimeter_t distanceBehind, const sim_mob::Agent* refAgent)
const
{
//	std::cout << "----------------------------" << std::endl;
	std::vector<Agent const *> results;
	if (impl_) {
		results = impl_->nearbyAgents(position, lane, distanceInFront, distanceBehind, refAgent);
	}

	return results;

}

/*std::vector<Agent const *> AuraManager::advanced_nearbyAgents(Point2D const & position, Lane const & lane, centimeter_t distanceInFront, centimeter_t distanceBehind, TreeItem* item) const {
	if (local_implType != IMPL_SIMTREE) {
		return nearbyAgents(position, lane, distanceInFront, distanceBehind);
	}

	std::vector<Agent const *> results;
	if (impl_) {
		results = impl_->advanced_nearbyAgents(position, lane, distanceInFront, distanceBehind, item);
	}

//	static long sum_count = 0;
//	sum_count += results.size();
//	if (sum_count % 100000 == 0)
//	std::cout << "advanced_nearbyAgents:" << results.size() << ",sum_count:" << sum_count << std::endl;

	return results;
}*/

/*void
AuraManager::printStatistics() const
{
    if (stats_)
    {
        stats_->printStatistics();
    }
    else
    {
        std::cout << "No statistics was collected by the AuraManager singleton." << std::endl;
    }
}*/

/**
 * xuyan
 */
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


