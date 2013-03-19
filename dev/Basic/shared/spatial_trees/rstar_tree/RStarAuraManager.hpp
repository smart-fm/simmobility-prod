#pragma once

#include "R_tree.hpp"
#include "metrics/Length.hpp"

namespace sim_mob
{

//Forward declarations.
class Point2D;
class Agent;
class Lane;


class RStarAuraManager
{
public:
	// No need to define the ctor and dtor.

	void update_rstar();

	std::vector<Agent const *>
	agentsInRect_rstar(Point2D const & lowerLeft, Point2D const & upperRight) const;

	std::vector<Agent const *>
	nearbyAgents_rstar(Point2D const & position, Lane const & lane, centimeter_t distanceInFront, centimeter_t distanceBehind) const;

private:
	sim_mob::R_tree tree_rstar;

	/* First dirty version... Will change eventually.
	 * This method is called from within the update of the AuraManager.
	 * This method increments the vehicle count for the road segment
	 * on which the Agent's vehicle is currently in.
	 */
//	void updateDensity(const Agent* ag);
};
}
