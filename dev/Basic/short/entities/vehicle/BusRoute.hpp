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

#include "geospatial/BusStop.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/RoadSegment.hpp"
#include "util/GeomHelpers.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob {

class PackageUtils;
class UnPackageUtils;



/**
 * A simple RoadSegment+percent offset for representing bus stops. See: BusRoute class.
 */
class DemoBusStop {
public:

	const sim_mob::RoadSegment* seg;
	double distance;
	double percent;
	double finalDist;
	const sim_mob::Point2D* position;

	bool operator== (const DemoBusStop& a) const
	{
	    return (a.seg==seg)&&(a.percent==percent);
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
			++currStop;
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
