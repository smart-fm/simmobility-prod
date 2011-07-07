#pragma once

#include <vector>

#include "RoadSegment.hpp"
#include "RoadItem.hpp"

namespace sim_mob
{

/**
 * A road or sidewalk. Generalized movement rules apply for agents inside a link,
 * which is itself composed of segments.
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class Link : public sim_mob::RoadItem {
public:
	int GetLength(bool isFwd) { return 0; }
	std::vector<sim_mob::RoadSegment*> GetPath(bool isFwd) { return std::vector<sim_mob::RoadSegment*>; }


protected:
	std::vector<sim_mob::RoadSegment*> segments;


};





}
