#pragma once

#include "Lane.hpp"
//#include "RoadItem.hpp"


namespace sim_mob
{


//Forward declarations
class RoadSegment;


/**
 * A lane for motorized vehicles. Links one Road Segment to another, by Lane ID.
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class LaneConnector {
public:
	const std::pair<const sim_mob::RoadSegment, unsigned int> laneFrom;
	const std::pair<const sim_mob::RoadSegment, unsigned int> laneTo;


private:



};





}
