/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "geospatial/RoadSegment.hpp"
#include "geospatial/StreetDirectory.hpp"


namespace sim_mob
{

/**
 * Simple class to handle movement along a string of RoadSegments
 *
 * \note
 * I'm making this header-only for now to make it simpler to develop. Will need to migrate to its own
 * separate *.cpp file if nothing breaks.
 */
class RoadSegmentMover {
public:
	RoadSegmentMover() {}

	//Start up
	void setPath(std::vector<sim_mob::WayPoint> a) {
		roadSegmentList.clear();
		for(std::vector<sim_mob::WayPoint>::iterator it=a.begin(); it!=a.end(); it++) {
			if(it->type_ == WayPoint::ROAD_SEGMENT) {
				roadSegmentList.push_back(it->roadSegment_);
			}
		}
		currSegmentIt = roadSegmentList.begin();
	}

	//Are we move-able?
	bool isPathSet() {
		return !roadSegmentList.empty();
	}

	//Advance
	bool moveToNextSegment() {
		if (currSegmentIt!=roadSegmentList.end()) {
			currSegmentIt++;
		}
		return currSegmentIt!=roadSegmentList.end();
	}

	//Retrieve
	const RoadSegment* getCurrSegment() {
		return *currSegmentIt;
	}



private:
	std::vector<const sim_mob::RoadSegment*> roadSegmentList;         //The vector we're moving along.
	std::vector<const sim_mob::RoadSegment*>::iterator currSegmentIt; //Our position in the list of segments.


};


}


