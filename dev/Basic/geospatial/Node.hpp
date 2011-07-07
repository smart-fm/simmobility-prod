#pragma once

#include <vector>

#include "Point2D.hpp"
#include "Link.hpp"
#include "LaneConnector.hpp"
#include "RoadSegment.hpp"

namespace sim_mob
{

/**
 * A location on a map where other elements interact. Nodes build off of the Point2D class
 * to include information on the Lane Connectors and RoadSegments at that particular location.
 */
class Node : public sim_mob::Point2D {
public:
	std::vector<sim_mob::LaneConnector*> getConnectors(const Link* from) { return std::vector<sim_mob::LaneConnector*>;};
	std::vector<sim_mob::RoadSegment*> getItemsAt() { return std::vector<sim_mob::RoadSegment*>; }



protected:
	std::vector<sim_mob::LaneConnector*> connectors;
	std::vector<sim_mob::RoadSegment*> itemsAt;



};





}
