/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <stdexcept>

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
	bool isPathSet() const {
		return !roadSegmentList.empty();
	}
	void throwIfPathUnset() const {
		if (!isPathSet()) {
			throw std::runtime_error("RoadSegmentMover path not set.");
		}
	}

	//Advance
	bool moveToNextSegment() {
		throwIfPathUnset();
		if (currSegmentIt!=roadSegmentList.end()) {
			currSegmentIt++;
		}
		return currSegmentIt!=roadSegmentList.end();
	}

	//Done?
	/*bool isAtEnd() const {
		throwIfPathUnset();
		return currSegmentIt == roadSegmentList.end();
	}*/
	bool isOnLastSegment() const {
		throwIfPathUnset();
		return currSegmentIt+1==roadSegmentList.end();
	}
	bool isOnFirstSegment() const {
		throwIfPathUnset();
		return currSegmentIt==roadSegmentList.begin();
	}

	//Retrieve
	const RoadSegment* getCurrSegment() {
		throwIfPathUnset();
		return *currSegmentIt;
	}
	const Link* getCurrLink() {
		throwIfPathUnset();
		return getCurrSegment()->getLink();
	}



private:
	//Movement within a Link
	std::vector<const sim_mob::RoadSegment*> roadSegmentList;
	std::vector<const sim_mob::RoadSegment*>::iterator currSegmentIt;





};


}


