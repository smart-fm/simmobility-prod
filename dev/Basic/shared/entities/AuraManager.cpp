/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <limits>
#include <algorithm>
#include <boost/unordered_set.hpp>

#include "Entity.hpp"
#include "Agent.hpp"
#include "AuraManager.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadSegment.hpp"
#include "buffering/Vector2D.hpp"
#include "entities/Person.hpp"

#include "spatial_trees/rstar_tree/RStarAuraManager.hpp"
#include "spatial_trees/simtree/SimAuraManager.hpp"
#include "spatial_trees/rdu_tree/RDUAuraManager.hpp"

namespace sim_mob
{
/* static */ AuraManager AuraManager::instance_;

/** \cond ignoreAuraManagerInnards -- Start of block to be ignored by doxygen.  */

////////////////////////////////////////////////////////////////////////////////////////////
// AuraManager::Stats
////////////////////////////////////////////////////////////////////////////////////////////

struct AuraManager::Stats : private boost::noncopyable
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

/** \endcond ignoreAuraManagerInnards -- End of block to be ignored by doxygen.  */

////////////////////////////////////////////////////////////////////////////////////////////
// AuraManager
////////////////////////////////////////////////////////////////////////////////////////////

void
AuraManager::init(bool keepStats /* = false */)
{
    if (keepStats)
        stats_ = new Stats;

    //Reset time tick.
    time_step = 0;

	if (choose_tree == RSTAR) {
		std::cout << "RSTAR" << std::endl;
		impl_ = new RStarAuraManager();
		impl_->init();
	} else if (choose_tree == SIMTREE) {
		std::cout << "SIMTREE" << std::endl;
		impl_ = new SimAuraManager();
		impl_->init();
	} else if (choose_tree == RDU) {
		std::cout << "RDU" << std::endl;
		impl_ = new RDUAuraManager();
		impl_->init();
	} else {
		throw std::runtime_error("Unknown tree type.");
	}
}

/* virtual */ void
AuraManager::update()
{
	PerformanceProfile::instance().markStartUpdate();
	if (impl_) {
		impl_->update(time_step);
	}

	time_step++;
	PerformanceProfile::instance().markEndUpdate();

	PerformanceProfile::instance().update();
}

std::vector<Agent const *>
AuraManager::agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight)
const
{
	PerformanceProfile::instance().markStartQuery(1);

	std::vector<Agent const *> results;
	if (impl_) {
		results = impl_->agentsInRect(lowerLeft, upperRight);
		PerformanceProfile::instance().markEndQuery(1);
	}
	return results;

}

std::vector<Agent const *>
AuraManager::nearbyAgents(Point2D const & position, Lane const & lane,
                          centimeter_t distanceInFront, centimeter_t distanceBehind)
const
{
	PerformanceProfile::instance().markStartQuery(1);


	std::vector<Agent const *> results;
	if (impl_) {
		results = impl_->nearbyAgents(position, lane, distanceInFront, distanceBehind);
		PerformanceProfile::instance().markEndQuery(1);
	}

	return results;

}

void
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
}

/**
 * xuyan
 */
void AuraManager::registerNewAgent(Agent const* one_agent)
{
//	std::cout << "Add 1." << std::endl;
	PerformanceProfile::instance().markStartUpdate();

	if (impl_) {
		if (dynamic_cast<Person const*>(one_agent)) {
			impl_->registerNewAgent(one_agent);
		}
	}

	PerformanceProfile::instance().markEndUpdate();
}

} // end of sim_mob


