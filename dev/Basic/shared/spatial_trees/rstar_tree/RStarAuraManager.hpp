#pragma once

#include "R_tree.hpp"
#include "metrics/Length.hpp"
#include "spatial_trees/TreeImpl.hpp"

namespace sim_mob
{

//Forward declarations.
class Point2D;
class Agent;
class Lane;


class RStarAuraManager : public TreeImpl
{
public:
	//No initialization required.
	virtual void init() {}

	virtual void update(int time_step);

	//Not used.
	virtual void registerNewAgent(const Agent* ag) {}

	virtual std::vector<Agent const *> agentsInRect(const Point2D& lowerLeft, const Point2D& upperRight) const;

	virtual std::vector<Agent const *> nearbyAgents(const Point2D& position, const Lane& lane, centimeter_t distanceInFront, centimeter_t distanceBehind) const;

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
