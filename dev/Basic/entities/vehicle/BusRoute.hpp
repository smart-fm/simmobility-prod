/* Copyright Singapore-MIT Alliance for Research and Technology */

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
#include "metrics/Frame.hpp"



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


	///Is there a bus stop on the current road segment?
	bool isBusStopOnCurrSegment(const RoadSegment* curr) const {
		return seg==curr;
	}

	///Have we reached this bus stop?
	bool atOrPastBusStop(const RoadSegment* curr, const double distTraveledOnSegmentZeroLane) const {
		const std::vector<Point2D>& poly = const_cast<RoadSegment*>(seg)->getLaneEdgePolyline(0);
		double totalDist = 0.0;
		int i;
		DynamicVector currSegmentLengthA(poly.end()->getX(), poly.end()->getY(), poly.begin()->getX(), poly.begin()->getY());
		for (std::vector<Point2D>::const_iterator it=poly.begin(); it!=poly.end(); it++) {
			if (it!=poly.begin()) {
				DynamicVector currSegmentLength(it->getX(), it->getY(), (it-1)->getX(), (it-1)->getY());
				totalDist += currSegmentLength.getMagnitude();
				std::cout<<"SURPRISE SURPRISE    "<<it->getX()<<"      "<<it->getY()<<"      "<<(it-1)->getX()<<"      "<<(it-1)->getY()<<std::endl;
				i++;
			}
			std::cout<<"POLYPOINTS ARE   "<<(poly.end()-1)->getX()<<" , "<<(poly.end()-1)->getY()<<" , "<<(poly.begin())->getX()<<" , "<< (poly.begin())->getY()<<std::endl;

		}

/*
		if (isBusStopOnCurrSegment(curr)) {
			std::cout <<"Test: " <<distTraveledOnSegmentZeroLane <<" => " <<percent*totalDist <<"   (" <<(isBusStopOnCurrSegment(curr) && (distTraveledOnSegmentZeroLane >= percent*totalDist)) <<")" <<"\n";
		}*/

		std::cout<<"Total Distance is     "<<totalDist<<std::endl;
		std::cout<<"Distance travelled on segment"<<distTraveledOnSegmentZeroLane<<"and distance else is"<<curr->length<<"      "<<percent<<"       "<<percent*totalDist<<"      "<<std::endl;
		if (percent>0){
		return (distTraveledOnSegmentZeroLane > percent*totalDist);
		}
		else return 0;

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

	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;
};

} // namespace sim_mob
