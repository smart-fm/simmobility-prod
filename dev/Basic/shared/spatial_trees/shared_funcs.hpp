#pragma once

#include <boost/unordered_set.hpp>

#include "metrics/Length.hpp"


namespace sim_mob {

//Forward declarations
class Agent;
class Entity;
class Lane;
class Point2D;
class RoadSegment;


namespace temp_spatial {  //Not sure what to call this namespace. ~Seth


// Return the squared eulidean distance between agent1 and agent2.
double distance(const sim_mob::Agent* agent1, const sim_mob::Agent* agent2);


// Find the agent in the <agents> collection that is closest to <agent>.
const sim_mob::Agent* nearest_agent(const sim_mob::Agent* agent, const boost::unordered_set<const sim_mob::Entity*>& agents);


// Return the sum of the width of <lane> and the width of the adjacent lanes on the left
// and right of <lane>.  If there is no lane on the left, add 300 centimeters.  Similarly
// if there is no lane on the right, add 300 centimeters.
sim_mob::centimeter_t getAdjacentLaneWidth(const sim_mob::Lane& lane);


// Return true if <point> is between <p1> and <p2>, even if <point> is not co-linear
// with <p1> and <p2>.
bool isInBetween(const sim_mob::Point2D& point, const sim_mob::Point2D& p1, const sim_mob::Point2D& p2);



// Adjust <p1> and <p2> so that <p2> is <distanceInFront> from <position> and
// <p1> is <distanceBehind> from <position>, while retaining the slope of the line
// from <p1> to <p2>.
void adjust(sim_mob::Point2D& p1, sim_mob::Point2D& p2, const sim_mob::Point2D& position, sim_mob::centimeter_t distanceInFront, sim_mob::centimeter_t distanceBehind);



}} //End namespace sim_mob::temp_spatial
