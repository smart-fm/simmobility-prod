//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <set>

#include "R_tree.hpp"
#include "metrics/Length.hpp"
#include "spatial_trees/TreeImpl.hpp"

namespace sim_mob
{

//Forward declarations.
class Point;
class Agent;
class WayPoint;

class RStarAuraManager : public TreeImpl
{
public:
	//Note: The pointers in removedAgentPointers will be deleted after this time tick; do *not*
	//      save them anywhere.
	virtual void update(int time_step, const std::set<sim_mob::Agent *> &removedAgentPointers);

	virtual std::vector<Agent const *> agentsInRect(const Point &lowerLeft, const Point &upperRight, const sim_mob::Agent *refAgent) const;

	virtual std::vector<Agent const *> nearbyAgents(const Point &position, const WayPoint &wayPoint, double distanceInFront, double distanceBehind,
													const sim_mob::Agent *refAgent) const;

private:
	sim_mob::R_tree tree_rstar;

};
}
