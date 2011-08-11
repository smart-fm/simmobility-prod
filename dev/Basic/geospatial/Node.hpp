/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <map>
#include <set>

namespace sim_mob
{


//Forward declarations
class Point2D;
class Link;
class Lane;
class LaneConnector;
class RoadSegment;

namespace aimsun
{
//Forward declarations
//class Loader;
} //End aimsun namespace


/**
 * A location on a map where other elements interact. Nodes contain a Point2D representing their
 * location. Additional information (such as lane connectors) are located in other classes (e.g.,
 * Intersections, Roundabouts, and SegmentNodes.
 *
 * The constructor of a node is protected, since there's never a need to use a Node directly (if only
 * the location is needed, a Point2D will do).
 */
class Node {
public:
	//This node's location.
	sim_mob::Point2D* location;


protected:
	Node() {}

//friend class sim_mob::aimsun::Loader;

};





}
