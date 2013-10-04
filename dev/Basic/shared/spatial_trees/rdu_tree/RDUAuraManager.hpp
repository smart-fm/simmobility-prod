//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <set>

#include "R_tree_DU.hpp"
#include "metrics/Length.hpp"
#include "spatial_trees/TreeImpl.hpp"


namespace sim_mob
{

//Forward declarations.
class Point2D;
class RoadSegment;
class Agent;
class Lane;



class RDUAuraManager : public TreeImpl
{
public:
	//Note: The pointers in removedAgentPointers will be deleted after this time tick; do *not*
	//      save them anywhere.
	virtual void update(int time_step, const std::set<sim_mob::Agent*>& removedAgentPointers);

	bool has_one_agent_du(int agent_id);

	virtual std::vector<Agent const *> agentsInRect(const Point2D& lowerLeft, const Point2D& upperRight, const sim_mob::Agent* refAgent) const;

	virtual std::vector<Agent const *> nearbyAgents(const Point2D& position, const Lane& lane, centimeter_t distanceInFront, centimeter_t distanceBehind, const sim_mob::Agent* refAgent) const;

private:
	sim_mob::R_tree_DU tree_du;
};

}
