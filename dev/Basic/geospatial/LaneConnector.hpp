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
 */
class LaneConnector {
public:
	const sim_mob::Lane* getLaneFrom() const {
		return laneFrom;
	}
	const sim_mob::Lane* getLaneTo() const {
		return laneTo;
	}

private:
	//NOTE: These items used to be const, but it's easier to declare them private and just
	//      return a const item.
	sim_mob::Lane* laneFrom;
	sim_mob::Lane* laneTo;


friend class sim_mob::aimsun::Loader;

};





}
