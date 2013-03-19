#pragma once

#include "SimRTree.h"

#include "metrics/Length.hpp"

namespace sim_mob
{

//Forward declaration.
class Point2D;
class Lane;
class Agent;

class SimAuraManager
{
public:
	// No need to define the ctor and dtor.

	void update_sim(int time_step);

	//xuyan: Build the Tree in the very beginning
	void init_sim();

	//xuyan:
	void registerNewAgent_sim(Agent const* one_agent);

	std::vector<Agent const *>
	agentsInRect_sim(Point2D const & lowerLeft, Point2D const & upperRight) const;

	std::vector<Agent const *>
	nearbyAgents_sim(Point2D const & position, Lane const & lane, centimeter_t distanceInFront, centimeter_t distanceBehind) const;

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
