//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <set>

#include "SimRTree.hpp"

#include "metrics/Length.hpp"
#include "spatial_trees/TreeImpl.hpp"

namespace sim_mob
{

class Agent;
class Point;
class WayPoint;

class SimAuraManager : public TreeImpl
{
public:
	/**
	 * Update all agents in the simulation.
	 * @param time_step simulation time_step
	 * @param removedAgentPointers temp comtainer
	 *
	 * The pointers in removedAgentPointers will be deleted after this time tick; do *not* save them anywhere.
	 */
	virtual void update(int time_step, const std::set<sim_mob::Agent *> &removedAgentPointers);

	/**
	 * Initialise the Sim-Tree using the Network Boundary (Locations of Multi-Nodes)
	 */
	virtual void init();

	/**
	 * Puts new agents into Sim-Tree; After that, Sim-Tree will auto manage the agent's locations and auto remove the agent
	 * when it finishes its trip
	 * 
	 * @param ag which agent to insert
	 */
	virtual void registerNewAgent(const Agent* ag);

	/**
	 * Return a collection of agents that are located in the axially-aligned rectangle.
	 * NOTE: The caller is responsible to determine the "type" of each agent in the returned array.
	 *
	 * @param lowerLeft The lower left corner of the axially-aligned search rectangle.
	 * @param upperRight The upper right corner of the axially-aligned search rectangle.
	 * @param refAgent The agent performing the query. If non-null, certain implementations (namely the Sim Tree)
	 * can make use of an optimised bottom-up query in some cases. If null, the algorithm used will always
	 * be the slower, top-down query.
	 * 
	 * @return a collection of agents
	 */
	virtual std::vector<Agent const *> agentsInRect(const Point &lowerLeft, const Point &upperRight, const sim_mob::Agent *refAgent) const;

	/**
	 * Return a collection of agents that are on the left, right, front, and back of the specified
	 * position.
	 * This query is designed for Driver/Vehicle agents. It calculates the search rectangle
	 * based on position, lane/turning, distanceInFront, and distanceBehind.  position
	 * should be the current location of the Driver agent and is within the boundary of lane/turning.
	 * The search rectangle is (not necessarily symmetrically) centered around position.
	 * It includes the adjacent lanes/turnings on the left and right of position (effectively of
	 * lane/turning as well). If lane/turning is the leftmost and/or is the rightmost, the search
	 * rectangle extends 300 centimetre to include the sidewalk or the road segment of the reverse
	 * direction.
	 * 
	 * @param position The centre of the search rectangle.
	 * @param wayPoint Holds the current lane or the turning
	 * @param distanceInFront The forward distance of the search rectangle.
	 * @param distanceBehind The back
	 * @param refAgent The agent performing the query. If non-null, certain implementations (namely the Sim Tree)
	 * can make use of an optimised bottom-up query in some cases. If null, the algorithm used will always
	 * be the slower, top-down query.
	 *
	 * @return a collection of agents
	 */
	virtual std::vector<Agent const *> nearbyAgents(const Point& position, const WayPoint& wayPoint, double distanceInFront, double distanceBehind,
													const sim_mob::Agent* refAgent) const;

	/**
	 * Return a collection of agents that are located in the axially-aligned rectangle.
	 * The function is the same with function agentsInRect(), but it is optimised for performance.
	 *
	 * @param lowerLeft The lower left corner of the axially-aligned search rectangle.
	 * @param upperRight The upper right corner of the axially-aligned search rectangle.
	 * @param refAgent The agent performing the query. If non-null, certain implementations (namely the Sim Tree)
	 * can make use of an optimised bottom-up query in some cases. If null, the algorithm used will always
	 * be the slower, top-down query.
	 *
	 * @return a collection of agents
	 */
	std::vector<Agent const *> agentsInRectBottomUpQuery(const Point& lowerLeft, const Point& upperRight, TreeItem* item) const;

	/**
	 * Return a collection of agents that are on the left, right, front, and back of the specified
	 * position.
	 * The function is the same with function nearbyAgents, but it is optimised for performance.
	 *
	 * @param position The centre of the search rectangle.
	 * @param wayPoint Holds the current lane or the turning
	 * @param distanceInFront The forward distance of the search rectangle.
	 * @param distanceBehind The back
	 * @param item The agent performing the query. If non-null, certain implementations (namely the Sim Tree)
	 * can make use of an optimised bottom-up query in some cases. If null, the algorithm used will always
	 * be the slower, top-down query.
	 *
	 * @return a collection of agents
	 */
	std::vector<Agent const *> nearbyAgentsBottomUpQuery(const Point &position, const WayPoint &wayPoint, double distanceInFront, double distanceBehind,
														TreeItem* item) const;

private:
	sim_mob::SimRTree tree_sim;

	//Add new agents each time step
	std::vector<Agent const *> new_agents;

private:
	//This used to be stored at the Agent level as "connector_to_Sim_Tree". It makes more sense here.
	//Note that all entries are non-null; if an entry does not exist, it is assumed to be null.
	//XUYAN: I am not sure where it is used. But I did not remove it.
	std::map<const sim_mob::Agent *, TreeItem *> agent_connector_map;

};

}
