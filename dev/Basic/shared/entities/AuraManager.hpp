//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <boost/utility.hpp>

#include "geospatial/network/WayPoint.hpp"
#include "metrics/Length.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob
{
class Entity;
class Agent;
class Point;
class TreeImpl;

struct TreeItem;

/**
 * A singleton that can locate agents/entities within any rectangle.
 *
 * \author LIM Fung Chai
 * \author Seth N. Hetu
 *
 * To locate nearby agents, calculate 2 points to form the search rectangle before calling
 * agentsInRect(). 
 *   \code
 *   Point p1 = ...;
 *   Point p2 = ...;
 *   const std::vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(p1, p2);
 *   \endcode
 *
 * Alternatively if an agent is located on a lane, it can call nearbyAgents(), passing in its
 * current position and lane, and the distance forward and behind its current position.
 *
 * Both methods return an array of Agent's.  It is the responsibility of the caller to determine
 * the type (i.e. the current role) of each agent.
 */
class AuraManager : private boost::noncopyable
{
public:
	/**Types of implementations supported.*/
	enum AuraManagerImplementation
	{
		/**R-Star tree*/
		IMPL_RSTAR,

		/**RDU tree*/
		IMPL_RDU
	};

	static AuraManager& instance()
	{
		return instance_;
	}

	/**
	 * Called every frame, this method builds a spatial index of the positions of all agents.
	 *
	 * This method should be called after all the agents have calculated their new positions
	 * and (if double-buffering data types are used) after the new positions are published.
	 *
	 * Note: The pointers in removedAgentPointers will be deleted after this time tick; do *not*
	 * save them anywhere.
	 */
	void update(const std::set<sim_mob::Entity *>& removedAgentPointers);

	/**
	 * Return a collection of agents that are located in the axially-aligned rectangle.
	 * 
	 * @param lowerLeft The lower left corner of the axially-aligned search rectangle.
	 * @param upperRight The upper right corner of the axially-aligned search rectangle.
	 * @param refAgent The agent performing the query. If non-null, certain implementations (namely the Sim Tree) can make use
	 * of an optimised bottom-up query in some cases. If null, the algorithm used will always be the slower, top-down query.
	 * 
	 * @return a collection of agents
	 *
	 * The caller is responsible to determine the "type" of each agent in the returned array.
	 */
	std::vector<Agent const *> agentsInRect(Point const &lowerLeft, Point const &upperRight, const sim_mob::Agent *refAgent) const;

	/**
	 * Return a collection of agents that are on the left, right, front, and back of the specified
	 * position.
	 * 
	 * This query is designed for Driver/Vehicle agents.It calculates the search rectangle
	 * based on position, way-point, distanceInFront, and distanceBehind. position
	 * should be the current location of the Driver agent and is within the boundary oflane.
	 * The search rectangle is (not necessarily symmetrically) centered around position.
	 * It includes the adjacent lanes on the left and right of position (effectively of
	 * lane as well). If lane is the leftmost lane and/or is the rightmost lane, the search
	 * rectangle extends 300 centimetre to include the sidewalk or the road segment of the reverse
	 * direction.
	 *
	 * @param position The centre of the search rectangle.
	 * @param wayPoint Holds the current lane or the turning
	 * @param distanceInFront The forward distance of the search rectangle.
	 * @param distanceBehind The back
	 * @param refAgent The agent performing the query. If non-null, certain implementations (namely the Sim-Tree) can make
	 * use of an optimised bottom-up query in some cases. If null, the algorithm used will always be the slower, top-down query.
	 * 
	 * @return a collection of agents
	 */
	std::vector<Agent const *> nearbyAgents(Point const &position, WayPoint const &wayPoint, double distanceInFront, double distanceBehind,
											const Agent *refAgent) const;

	/**
	 * Initialise the AuraManager object (to be invoked by the simulator kernel).
	 *
	 * @param keepStats Keep statistics on internal operations if true.
	 */
	void init(AuraManagerImplementation implType);

	/**
	 * Destroy the object implementing the AuraManager
	 */
	void destroy();

	/**
	 * Register new agents to AuraManager each time step
     * @param one_agent agent to be registered
     */
	void registerNewAgent(Agent const *one_agent);

private:
	AuraManager() : impl_(nullptr), time_step(0)
	{
	}

	static AuraManager instance_;

	//Current implementation being used (via inheritance).
	TreeImpl *impl_;

	//Current time step.
	int time_step;
};

}
