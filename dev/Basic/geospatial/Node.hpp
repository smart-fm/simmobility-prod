/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <map>
#include <set>

#include "util/LangHelpers.hpp"
#include "util/OpaqueProperty.hpp"

namespace sim_mob
{


//Forward declarations
class Point2D;
class Link;
class Lane;
class LaneConnector;
class RoadSegment;


/**
 * A location on a map where other elements interact. Nodes contain a Point2D representing their
 * location. Additional information (such as lane connectors) are located in other classes (e.g.,
 * Intersections, Roundabouts, and SegmentNodes.
 *
 * Nodes should not be constructed directly. Instead, they exist to provide a uniform interface
 * to define locations in a RoadNetwork. UniNodes and MultiNodes (and their subclasses) provide
 * more comprehensive functionality, and their sub-classes provide even more.
 */
class Node {
public:
	virtual ~Node() {} //A virtual destructor allows dynamic casting

	///The location of this Node.
	sim_mob::Point2D* location;

	//Nodes may have hidden properties useful only in for the visualizer.
	OpaqueProperty<int> originalDB_ID;

protected:
	Node() : location(nullptr) {}

};



}
