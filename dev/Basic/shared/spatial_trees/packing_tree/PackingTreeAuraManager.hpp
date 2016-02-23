//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <vector>

#include "entities/Agent.hpp"
#include "spatial_trees/TreeImpl.hpp"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

namespace sim_mob
{

typedef bg::model::point<float, 2, bg::cs::cartesian> point;
typedef bg::model::box<point> box;
typedef std::pair<box, const Agent *> value;

class PackingTreeAuraManager : public TreeImpl
{
private:
	/**The r-star tree with packing insert algorithm*/
	bgi::rtree<value, bgi::rstar<300000> > *tree;

public:
	PackingTreeAuraManager();
	virtual ~PackingTreeAuraManager();

	/**
	 * Update all agents in the simulation.
	 * 
	 * @param time_step simulation time_step
	 * @param removedAgentPointers temp container
	 *
	 * The pointers in removedAgentPointers will be deleted after this time tick; do *not* save them anywhere.
	 */
	virtual void update(int time_step, const std::set<sim_mob::Entity *> &removedAgentPointers);

	/**
	 * Return a collection of agents that are located in the axially-aligned rectangle.
	 * 
	 * @param lowerLeft The lower left corner of the axially-aligned search rectangle.
	 * @param upperRight The upper right corner of the axially-aligned search rectangle.
	 * @param refAgent The agent performing the query. If non-null, certain implementations
	 * (namely the Sim Tree) can make use of an optimized bottom-up query in some cases.
	 * If null, the algorithm used will always be the slower, top-down query.
	 * 
	 * @return a collection of agents
	 * The caller is responsible to determine the "type" of each agent in the returned array.
	 */
	virtual std::vector<Agent const *> agentsInRect(const Point &lowerLeft, const Point &upperRight, const sim_mob::Agent *refAgent) const;

	/**
	 * Return a collection of agents that are on the left, right, front, and back of the specified
	 * position.
	 * 
	 * @param position The center of the search rectangle.
	 * @param wayPoint The wapypoint (lane or turning path)
	 * @param distanceInFront The forward distance of the search rectangle.
	 * @param distanceBehind The backward distance of the search rectangle
	 * @param refAgent The agent performing the query. If non-null, certain implementations
	 * (namely the Sim Tree) can make use of an optimized bottom-up query in some cases.
	 * If null, the algorithm used will always be the slower, top-down query.
	 * 
	 * @return a collection of agents
	 * This query is designed for Driver/Vehicle agents.  It calculates the search rectangle
	 * based on position, lane, distanceInFront, and distanceBehind. position
	 * should be the current location of the Driver agent and is within the boundary of lane.
	 * The search rectangle is (not necessarily symmetrically) centered around position.
	 * It includes the adjacent lanes on the left and right of position (effectively of
	 * lane as well).  If lane is the leftmost lane and/or is the rightmost lane, the search
	 * rectangle extends 300 centimeters to include the sidewalk or the road segment of the reverse
	 * direction.
	 */
	virtual std::vector<Agent const *> nearbyAgents(const Point &position, const WayPoint &wayPoint, double distanceInFront, double distanceBehind,
													const sim_mob::Agent *refAgent) const;

};

}
