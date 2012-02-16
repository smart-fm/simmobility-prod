/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file BusRoute.hpp
 *
 * \author Seth N. Hetu
 */

#pragma once

#include <vector>

#include "geospatial/RoadSegment.hpp"
#include "util/LangHelpers.hpp"
#include "util/GeomHelpers.hpp"
#include "metrics/Frame.hpp"



namespace sim_mob {

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif



/**
 * A simple RoadSegment+percent offset for representing bus stops. See: BusRoute class.
 */
class DemoBusStop {
public:
	const sim_mob::RoadSegment* seg;
	double percent;

	///Is there a bus stop on the current road segment?
	bool isBusStopOnCurrSegment(const RoadSegment* curr) const {
		return seg==curr;
	}

	///Have we reached this bus stop?
	bool atOrPastBusStop(const RoadSegment* curr, const double distTraveledOnSegmentZeroLane) const {
		const std::vector<Point2D>& poly = const_cast<RoadSegment*>(seg)->getLaneEdgePolyline(0);
		double totalDist = 0.0;
		for (std::vector<Point2D>::const_iterator it=poly.begin(); it!=poly.end(); it++) {
			if (it!=poly.begin()) {
				totalDist += sim_mob::dist(*it, *(it-1));
			}
		}

		//std::cout <<"Test: " <<distTraveledOnSegmentZeroLane <<" => " <<totalDist <<"\n";
		//throw 1;

		return isBusStopOnCurrSegment(curr) && (distTraveledOnSegmentZeroLane >= percent*totalDist);
	}
};

/**
 * A bus route defines how a bus traverses the road network. It consists of the waypoint path
 *  used to actually travel the road network and a set of "stops". For now, we define a stop
 *  as simply a percent distance between two nodes. Later, we can use actual structures to
 *  represent bus stops (and curbside stopping).
 */
class BusRoute {
public:
	/*BusRoute() : stops(std::vector<BusStop>()) { //No route; still allowed.
		reset();
	}*/

	explicit BusRoute(std::vector<DemoBusStop> stops=std::vector<DemoBusStop>()) : stops(stops) {
		//Start driving at the beginning
		reset();
	}


	void reset() {
		currStop = stops.begin();
	}
	void advance() {
		if (currStop!=stops.end()) {
			currStop++;
		}
	}
	const DemoBusStop* getCurrentStop() const {
		if (currStop!=stops.end()) {
			return &(*currStop);
		}
		return nullptr;
	}


private:
	std::vector<DemoBusStop> stops;
	std::vector<DemoBusStop>::const_iterator currStop;


#ifndef SIMMOB_DISABLE_MPI
public:
	friend class sim_mob::PackageUtils;
	friend class sim_mob::UnPackageUtils;
#endif
};

} // namespace sim_mob
