/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <set>

namespace sim_mob
{


//Forward declarations
class Point2D;
class Link;
class RoadSegment;
class LaneConnector;

namespace aimsun
{
//Forward declarations
class Loader;
}


/**
 * A location on a map where other elements interact. Nodes contain a Point2D representing their location,
 * and include information on the Lane Connectors and RoadSegments at that particular location.
 */
class Node {
public:

	///Query the list of connectors at the current node, restricting the results to
	//   those which originate at the "from" Link.
	std::vector<sim_mob::LaneConnector*> getConnectors(const sim_mob::Link* from) const;

	///Retrieve a list of all RoadSegments at this node.
	const std::set<sim_mob::RoadSegment*>& getRoadSegments() const { return itemsAt; }

	//This node's location.
	sim_mob::Point2D* location;


protected:
	//std::multimap<sim_mob::RoadSegment*, sim_mob::LaneConnector*> connectors;
	std::vector<sim_mob::LaneConnector*> connectors;
	std::set<sim_mob::RoadSegment*> itemsAt;


friend class sim_mob::aimsun::Loader;

};





}
