//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "SimRTree.h"

#include "metrics/Length.hpp"
#include "spatial_trees/TreeImpl.hpp"

namespace sim_mob
{

//Forward declaration.
class Point2D;
class Lane;
class Agent;

class SimAuraManager : public TreeImpl
{
public:
	// No need to define the ctor and dtor.

	virtual void update(int time_step);

	//xuyan: Build the Tree in the very beginning
	virtual void init();

	//xuyan:
	virtual void registerNewAgent(const Agent* ag);

	virtual std::vector<Agent const *> agentsInRect(const Point2D& lowerLeft, const Point2D& upperRight) const;

	virtual std::vector<Agent const *> nearbyAgents(const Point2D& position, const Lane& lane, centimeter_t distanceInFront, centimeter_t distanceBehind) const;

public:
	//	long sum_counts;
//	int first_update;

	sim_mob::SimRTree tree_sim;

	//xuyan: add new agents each time step
	std::vector<Agent const*> new_agents;

	/* First dirty version... Will change eventually.
	 * This method is called from within the update of the AuraManager.
	 * This method increments the vehicle count for the road segment
	 * on which the Agent's vehicle is currently in.
	 */
//	void updateDensity(const Agent* ag);

	//
	void checkLeaf();

};

}
