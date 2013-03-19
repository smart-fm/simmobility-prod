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
		pimpl_rstar = new RStarAuraManager();
	} else if (choose_tree == SIMTREE) {
		std::cout << "SIMTREE" << std::endl;
		pimpl_sim = new SimAuraManager();
		pimpl_sim->init_sim();
	} else if (choose_tree == RDU) {
		std::cout << "RDU" << std::endl;
		pimpl_du = new RDUAuraManager();
	}
}

/* virtual */ void
AuraManager::update()
{
	PerformanceProfile::instance().markStartUpdate();
	if (choose_tree == RSTAR) {
		if (pimpl_rstar)
			pimpl_rstar->update_rstar();
	} else if (choose_tree == SIMTREE) {
		if (pimpl_sim)
			pimpl_sim->update_sim(time_step);
	} else if (choose_tree == RDU) {
		if (pimpl_du)
			pimpl_du->update_du();
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

	if (choose_tree == RSTAR) {
		std::vector<Agent const *> results = pimpl_rstar->agentsInRect_rstar(lowerLeft, upperRight);
		PerformanceProfile::instance().markEndQuery(1);
		return results;
	} else if (choose_tree == SIMTREE) {
		std::vector<Agent const *> results = pimpl_sim->agentsInRect_sim(lowerLeft, upperRight);
		PerformanceProfile::instance().markEndQuery(1);
		return results;
	} else {
		std::vector<Agent const *> results = pimpl_du->agentsInRect_du(lowerLeft, upperRight);
		PerformanceProfile::instance().markEndQuery(1);
		return results;
	}

	//	if (results.size() != results_2.size())
	//		std::cout << "agentsInRect:" << results.size() << "," << results_2.size() << std::endl;

	//	std::cout << "BBB2" << std::endl;

	//	PerformanceProfile::instance().markEndQuery(thread_id);
}

std::vector<Agent const *>
AuraManager::nearbyAgents(Point2D const & position, Lane const & lane,
                          centimeter_t distanceInFront, centimeter_t distanceBehind)
const
{
	PerformanceProfile::instance().markStartQuery(1);

	if (choose_tree == RSTAR) {
		std::vector<Agent const *> results = pimpl_rstar->nearbyAgents_rstar(position, lane, distanceInFront, distanceBehind);
		PerformanceProfile::instance().markEndQuery(1);
		return results;
	} else if (choose_tree == SIMTREE) {
		std::vector<Agent const *> results = pimpl_sim->nearbyAgents_sim(position, lane, distanceInFront, distanceBehind);
		PerformanceProfile::instance().markEndQuery(1);
		return results;
	} else {
		std::vector<Agent const *> results = pimpl_du->nearbyAgents_du(position, lane, distanceInFront, distanceBehind);
		PerformanceProfile::instance().markEndQuery(1);
		return results;
	}

	//	std::cout << "Size:" << results.size() << "," << results_2.size() << std::endl;

	//	if (results.size() != results_2.size()) {
	//
	////		pimpl_sim->tree_sim.display();
	//
	//		std::cout << "Size:" << results.size() << "," << results_2.size() << std::endl;
	//
	//		std::vector<Agent const *>::iterator ite = results.begin();
	//		while (ite != results.end()) {
	//			std::cout << "results:" << (*ite)->xPos << "," << (*ite)->yPos << "," << (*ite)->getId() << std::endl;
	//			ite++;
	//		}
	//
	//		std::vector<Agent const *>::iterator ite2 = results_2.begin();
	//		while (ite2 != results_2.end()) {
	//			std::cout << "results_2:" << (*ite2)->xPos << "," << (*ite2)->yPos << "," << (*ite2)->getId() << std::endl;
	//			ite2++;
	//		}
	//	}

	//	std::cout << "AAA2" << std::endl;
	//	return pimpl_ ? pimpl_->nearbyAgents(position, lane, distanceInFront, distanceBehind) : std::vector<Agent const *>();

	//	return NULL;
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

	if (choose_tree == SIMTREE) {
		Person const* person = dynamic_cast<Person const*>(one_agent);
		if (person) {
//		std::cout << "Agent:xPos" << one_agent->xPos.get();
//		std::cout << ",Agent:yPos" << one_agent->yPos.get() << std::endl;

			pimpl_sim->registerNewAgent_sim(one_agent);
		}
	}

	PerformanceProfile::instance().markEndUpdate();
}

} // end of sim_mob


