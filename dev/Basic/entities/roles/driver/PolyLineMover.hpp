/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <stdexcept>

#include "geospatial/Point2D.hpp"
#include "util/DynamicVector.hpp"


namespace sim_mob
{

/**
 * Simple class to handle movement along a string of PolyPoints. Similar to RoadSegmentMover
 *
 * \note
 * I'm making this header-only for now to make it simpler to develop. Will need to migrate to its own
 * separate *.cpp file if nothing breaks.
 */
class PolyLineMover {
public:
	PolyLineMover() {}

	//Start up
	void setPath(const std::vector<sim_mob::Point2D>& a) {
		polypointsList.clear();
		for(std::vector<sim_mob::Point2D>::const_iterator it=a.begin(); it!=a.end(); it++) {
			polypointsList.push_back(*it);
		}

		currPolypoint = polypointsList.begin();
		nextPolypoint = polypointsList.begin();
		nextPolypoint++;
	}

	//Are we move-able?
	bool isPathSet() const {
		return !polypointsList.empty();
	}
	void throwIfPathUnset() const {
		if (!isPathSet()) {
			throw std::runtime_error("PolyLineMover path not set.");
		}
	}


	//Advance
	bool moveToNextSegment() {
		throwIfPathUnset();
		if (nextPolypoint!=polypointsList.end()) {
			currPolypoint++;
			nextPolypoint++;
		}
		return nextPolypoint!=polypointsList.end();
	}

	//Done?
	/*bool isAtEnd() const {
		throwIfPathUnset();
		return nextPolypoint==polypointsList.end();
	}*/
	bool isOnLastLine() const {
		throwIfPathUnset();
		return nextPolypoint+1==polypointsList.end();
	}

	//Retrieve
	std::pair<sim_mob::Point2D, sim_mob::Point2D> getCurrPolyline() {
		throwIfPathUnset();
		return std::make_pair(*currPolypoint, *nextPolypoint);
	}
	sim_mob::Point2D getCurrPolypoint() {
		throwIfPathUnset();
		return *currPolypoint;
	}
	sim_mob::Point2D getNextPolypoint() {
		throwIfPathUnset();
		return *nextPolypoint;
	}
	double getCurrPolylineLength() { //Retrieve the total length of all polylines in this polypoint array.
		double res = 0;
		for(size_t i=0; i<polypointsList.size()-1; i++) {
			DynamicVector temp(
				polypointsList.at(i).getX(), polypointsList.at(i).getY(),
				polypointsList.at(i+1).getX(), polypointsList.at(i+1).getY()
			);
			res += temp.getMagnitude();
		}
		return res;
	}


private:
	//Movement within a Link
	std::vector<sim_mob::Point2D> polypointsList;
	std::vector<sim_mob::Point2D>::iterator currPolypoint;
	std::vector<sim_mob::Point2D>::iterator nextPolypoint;





};


}


