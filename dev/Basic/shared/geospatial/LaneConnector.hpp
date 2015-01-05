//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "util/LangHelpers.hpp"
#include "geospatial/Lane.hpp"

//namespace geo {
//class connector_t_pimpl;
//class UniNode_t_pimpl;
//class GeoSpatial_t_pimpl;
//}

namespace sim_mob
{


//Forward declarations
class RoadSegment;
class Lane;


namespace aimsun
{
//Forward declaration
class Loader;
} //End namespace aimsun


/**
 * A lane for motorized vehicles. Links one Road Segment to another, by Lane ID.
 * \author Seth N. Hetu
 */
class LaneConnector {
public:
	explicit LaneConnector(sim_mob::Lane* from=nullptr, sim_mob::Lane* to=nullptr) : laneFrom(from), laneTo(to) {}

	const sim_mob::Lane* getLaneFrom() const {
		return laneFrom;
	}
	const sim_mob::Lane* getLaneTo() const {
		return laneTo;
	}

	/** Return the polyline of the lane connector  */
	std::vector<sim_mob::Point2D>& getPolyline() {
		if (polylinePoints.empty()) {
			// get from lane last polyline point and
			// to lane's first polyline point
			polylinePoints.push_back(laneFrom->getPolyline().back());
			polylinePoints.push_back(laneTo->getPolyline().front());
		}

		return polylinePoints;
	}

private:
	//NOTE: These items used to be const, but it's easier to declare them private and just
	//      return a const item.
	sim_mob::Lane* laneFrom;
	sim_mob::Lane* laneTo;

	/// lane connector polyline points
	std::vector<Point2D> polylinePoints;


friend class sim_mob::aimsun::Loader;


};





}
