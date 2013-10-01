//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file BusRoute.hpp
 *
 * \author Seth N. Hetu
 */

#pragma once

#include <vector>

#include "geospatial/RoadSegment.hpp"
#include "geospatial/Point2D.hpp"
#include "util/LangHelpers.hpp"
#include "util/GeomHelpers.hpp"
#include "geospatial/BusStop.hpp"


namespace sim_mob {

class PackageUtils;
class UnPackageUtils;



/**
 * A simple RoadSegment+percent offset for representing bus stops. See: BusRoute class.
 */
class DemoBusStop {
public:
	//DemoBusStop(RoadSegment *rs){};
	const sim_mob::RoadSegment* seg;
	double distance;
	double percent;
	double finalDist;
	const sim_mob::Point2D* position;

	bool operator== (const DemoBusStop& a) const
	{
	    return (a.seg==seg)&&(a.percent==percent);
	}


	///Is there a bus stop on the current road segment?
	bool isBusStopOnCurrSegment(const RoadSegment* curr) const {
		typedef std::map<centimeter_t, const RoadItem*>::const_iterator  ObstacleIterator;

		//Scan the obstacles list; return true if any RoadItem on it is a BusStop.
		for(ObstacleIterator o_it = curr->obstacles.begin(); o_it != curr->obstacles.end() ; o_it++) {
			if(dynamic_cast<const BusStop*>(o_it->second)) {
				return true;
			}
		}
		return false;
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

		std::cout<<"atOrPastBusStop: isBusStopOnCurrSegment <"<<isBusStopOnCurrSegment(curr)<<">"
				 <<" percent<"<<percent<<">"<<std::endl;
		return isBusStopOnCurrSegment(curr) && percent>0;
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

	const std::vector<DemoBusStop> getStops() const
	{
		return stops;
	}


private:
	std::vector<DemoBusStop> stops;
	std::vector<DemoBusStop>::const_iterator currStop;

	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;
};

} // namespace sim_mob
