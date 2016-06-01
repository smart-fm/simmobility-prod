//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/unordered_set.hpp>

#include "metrics/Length.hpp"
#include "geospatial/network/WayPoint.hpp"

namespace sim_mob
{

//Forward declarations
class Agent;
class Entity;
class Lane;
class Point;
class RoadSegment;

namespace spatial
{ 

/**
 * @param agent1
 * @param agent2
 * @return the squared euclidean distance between agent1 and agent2.
 */
double distance(const sim_mob::Agent *agent1, const sim_mob::Agent *agent2);

/**
 * Find the agent in the collection of agents that is closest to the given agent.
 *
 * @param agent the agent to be searched around
 * @param agents the collection of agents
 *
 * @return the agent in the collection of agents that is closest to the given agent
 */
const sim_mob::Agent* nearest_agent(const sim_mob::Agent *agent, const boost::unordered_set<const sim_mob::Entity *> &agents);

/**
 * Calculates the sum of the width of lane/turning path and the width of the adjacent lanes/turning paths on the
 * left and right of the given lane/turning path.
 * If there is no lane on the left, add 0.3 metre. Similarly if there is no lane on the right, add 0.3 metre.
 *
 * @param wayPoint holds the lane or the turning path
 *
 * @return the sum of the width of lane/turning path and the width of the adjacent lanes/turning paths
 */
double getAdjacentPathWidth(const sim_mob::WayPoint &wayPoint);

// Return true if <point> is between <p1> and <p2>, even if <point> is not co-linear
// with <p1> and <p2>.
bool isInBetween(const sim_mob::Point &point, const sim_mob::Point &p1, const sim_mob::Point &p2);

// Adjust <p1> and <p2> so that <p2> is <distanceInFront> from <position> and
// <p1> is <distanceBehind> from <position>, while retaining the slope of the line
// from <p1> to <p2>.
void adjust(sim_mob::Point &p1, sim_mob::Point &p2, const sim_mob::Point &position, double distanceInFront, double distanceBehind);

}
} 
