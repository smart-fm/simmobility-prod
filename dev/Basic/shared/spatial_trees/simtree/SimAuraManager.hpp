//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <set>

#include "SimRTree.hpp"

#include "metrics/Length.hpp"
#include "spatial_trees/TreeImpl.hpp"

namespace sim_mob {

//Forward declaration.
class Point2D;
class Lane;
class Agent;

class SimAuraManager: public TreeImpl {
public:
	//Note: The pointers in removedAgentPointers will be deleted after this time tick; do *not*
	//      save them anywhere.
	virtual void update(int time_step, const std::set<sim_mob::Agent*>& removedAgentPointers);

	//Build the Tree in the very beginning
	virtual void init();

	//put new agents into Sim-Tree;
	//After that, Sim-Tree will auto manage the agent's locations and auto remove the agent when it finishes its trip
	virtual void registerNewAgent(const Agent* ag);

	/**
	 * Return a collection of agents that are located in the axially-aligned rectangle.
	 *   \param lowerLeft The lower left corner of the axially-aligned search rectangle.
	 *   \param upperRight The upper right corner of the axially-aligned search rectangle.
	 *   \param refAgent The agent performing the query. If non-null, certain implementations
	 *          (namely the Sim Tree) can make use of an optimized bottom-up query in some cases.
	 *          If null, the algorithm used will always be the slower, top-down query.
	 *
	 * The caller is responsible to determine the "type" of each agent in the returned array.
	 */
	virtual std::vector<Agent const *> agentsInRect(const Point2D& lowerLeft, const Point2D& upperRight, const sim_mob::Agent* refAgent) const;

    /**
     * Return a collection of agents that are on the left, right, front, and back of the specified
     * position.
     *   \param position The center of the search rectangle.
     *   \param lane The lane
     *   \param distanceInFront The forward distance of the search rectangle.
     *   \param distanceBehind The back
     *   \param refAgent The agent performing the query. If non-null, certain implementations
     *          (namely the Sim Tree) can make use of an optimized bottom-up query in some cases.
     *          If null, the algorithm used will always be the slower, top-down query.
     *
     * This query is designed for Driver/Vehicle agents.  It calculates the search rectangle
     * based on \c position, \c lane, \c distanceInFront, and \c distanceBehind.  \c position
     * should be the current location of the Driver agent and is within the boundary of \c lane.
     * The search rectangle is (not necessarily symmetrically) centered around \c position.
     * It includes the adjacent lanes on the left and right of \c position (effectively of \c
     * lane as well).  If \c lane is the leftmost lane and/or is the rightmost lane, the search
     * rectangle extends 300 centimeters to include the sidewalk or the road segment of the reverse
     * direction.
     */
	virtual std::vector<Agent const *> nearbyAgents(const Point2D& position, const Lane& lane, centimeter_t distanceInFront, centimeter_t distanceBehind, const sim_mob::Agent* refAgent) const;

	/**
	 * Bottom-Up Query
	 */
	//Given the TreeItem the agent's standing on. This information is used to find a better start query point
	std::vector<Agent const *> agentsInRectBottomUpQuery(const Point2D& lowerLeft, const Point2D& upperRight, TreeItem* item) const;
	std::vector<Agent const *> nearbyAgentsBottomUpQuery(const Point2D& position, const Lane& lane, centimeter_t distanceInFront, centimeter_t distanceBehind, TreeItem* item) const;

private:
	sim_mob::SimRTree tree_sim;

	//Add new agents each time step
	std::vector<Agent const*> new_agents;

private:
	//This used to be stored at the Agent level as "connector_to_Sim_Tree". It makes more sense here.
	//Note that all entries are non-null; if an entry does not exist, it is assumed to be null.
	//XUYAN: I am not sure where it is used. But I did not remove it.
	std::map<const sim_mob::Agent*, TreeItem*> agent_connector_map;

};

}
