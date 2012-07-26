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
		bool isStop = false;
		const std::map<centimeter_t, const RoadItem*> & obstacles = curr->obstacles;
		for(std::map<centimeter_t, const RoadItem*>::const_iterator o_it = obstacles.begin(); o_it != obstacles.end() ; o_it++)
		{
		        		RoadItem* ri = const_cast<RoadItem*>(o_it->second);
		//
		        		BusStop *bs = dynamic_cast<BusStop *>(ri);
		        		if(bs)
		        			isStop=true;
		}
		return isStop;
	}

	///Have we reached this bus stop?
	///Have we reached this bus stop?
		bool atOrPastBusStop(const RoadSegment* curr, const double distTraveledOnSegmentZeroLane) const {
			const std::vector<Point2D>& poly = const_cast<RoadSegment*>(seg)->getLaneEdgePolyline(0);
			double totalDist = 0.0;
			for (std::vector<Point2D>::const_iterator it=poly.begin(); it!=poly.end(); it++) {
				if (it!=poly.begin()) {
					totalDist += sim_mob::dist(*it, *(it-1));
				}
			}

			/*if (isBusStopOnCurrSegment(curr)) {
				std::cout <<"Test: " <<distTraveledOnSegmentZeroLane <<" => " <<percent*totalDist <<"   (" <<(isBusStopOnCurrSegment(curr) && (distTraveledOnSegmentZeroLane >= percent*totalDist)) <<")" <<"\n";
			}*/
//std::cout<<"CEILING"<<percent<<ceil(percent)<<std::endl;
//if(ceil(percent))
//{
			//std::cout<<"OSTACLES"<<<<"HELLO"<<std::endl;

			std::cout<<"atOrPastBusStop: isBusStopOnCurrSegment <"<<isBusStopOnCurrSegment(curr)<<">"
					<<" percent<"<<percent<<">"<<std::endl;
			return isBusStopOnCurrSegment(curr) && percent>0;



			                               // else return 0;
//}

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
		//Start driving at the beginningx
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
