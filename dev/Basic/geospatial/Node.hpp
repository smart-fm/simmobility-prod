#pragma once

#include <vector>

#include "Point2D.hpp"
//#include "Link.hpp"
//#include "LaneConnector.hpp"
//#include "RoadSegment.hpp"

namespace sim_mob
{


//Forward declarations
class Link;
class RoadSegment;
class LaneConnector;


/**
 * A location on a map where other elements interact. Nodes build off of the Point2D class
 * to include information on the Lane Connectors and RoadSegments at that particular location.
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class Node : public sim_mob::Point2D {
public:
	std::vector<sim_mob::LaneConnector*> getConnectors(const sim_mob::Link* from) { return std::vector<sim_mob::LaneConnector*>();}
	std::vector<sim_mob::RoadSegment*> getItemsAt() { return std::vector<sim_mob::RoadSegment*>(); }



protected:
	std::vector<sim_mob::LaneConnector*> connectors;
	std::vector<sim_mob::RoadSegment*> itemsAt;



};





}
