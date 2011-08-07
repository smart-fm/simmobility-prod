/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <set>

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

namespace aimsun
{
//Forward declarations
class Loader;
}


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
	std::vector<sim_mob::LaneConnector*> getConnectors(const sim_mob::Link* from) const;
	std::set<sim_mob::RoadSegment*> getItemsAt() const;


protected:
	std::vector<sim_mob::LaneConnector*> connectors;
	std::set<sim_mob::RoadSegment*> itemsAt;


friend class sim_mob::aimsun::Loader;

};





}
