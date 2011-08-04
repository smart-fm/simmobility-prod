/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "Lane.hpp"
//#include "RoadItem.hpp"


namespace sim_mob
{


//Forward declarations
class RoadSegment;


namespace aimsun
{
//Forward declaration
class Loader;
} //End namespace aimsun


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
	const std::pair<sim_mob::RoadSegment*, unsigned int>& getLaneFrom() const;
	const std::pair<sim_mob::RoadSegment*, unsigned int>& getLaneTo() const;

private:
	//TODO: These items used to be const, but it's easier to declare them private and just
	//      return a const item.
	std::pair<sim_mob::RoadSegment*, unsigned int> laneFrom;
	std::pair<sim_mob::RoadSegment*, unsigned int> laneTo;


friend class sim_mob::aimsun::Loader;

};





}
