/* Copyright Singapore-MIT Alliance for Research and Technology */
/*
 * BusTrip.hpp
 *
 *  Created on: Sep 16, 2012
 *      Author: Yao Jin
 */

#pragma once

#include <vector>
#include <string>

#include "TripChain.hpp"
#include "geospatial/BusStop.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

namespace sim_mob {

struct BusStop_Times{
	explicit BusStop_Times(std::string scheduled_ArrivalTime, std::string scheduled_DepartureTime,
			std::string real_ArrivalTime = "00:00:00", std::string real_DepartureTime = "00:00:00");
	sim_mob::DailyTime scheduled_ArrivalTime;
	sim_mob::DailyTime scheduled_DepartureTime;
	sim_mob::DailyTime real_ArrivalTime;// real Arrival Time
	sim_mob::DailyTime real_DepartureTime;// real Departure Time
};

class BusStopInfo { // not clear
public:
	BusStopInfo();
	virtual ~BusStopInfo() {}
private:
	int stop_id;
	std::string stop_name;
	int roadsegment_id;
	BusStop_Times busStop_times;// for each particular BusTrip with this stop_id
};

class BusRouteInfo { // need copy constructor since BusTrip copy the BusRoute, or may need assign constructor
public:
	BusRouteInfo(unsigned int busRoute_id=0);
	BusRouteInfo(const BusRouteInfo& copyFrom); ///<Copy constructor
	virtual ~BusRouteInfo() {}
	const std::vector<const sim_mob::RoadSegment*>& getRoadSegments() const {
		return roadSegment_vec;
	}
	const std::vector<const sim_mob::BusStopInfo*>& getBusStopsInfo() const {
		return busStopInfo_vec;
	}
	void addBusStop(const sim_mob::BusStop* aBusStop);
	void addRoadSegment(const sim_mob::RoadSegment* aRoadSegment);
	void addBusStopInfo(const sim_mob::BusStopInfo* aBusStopInfo);
private:
	unsigned int busRoute_id;
	std::vector<const sim_mob::RoadSegment*> roadSegment_vec;
	std::vector<const sim_mob::BusStopInfo*> busStopInfo_vec;
	std::vector<const sim_mob::BusStop*> busStop_vec;
};

class BusTrip: public sim_mob::Trip {
public:
	BusTrip(int entId=0, std::string type="BusTrip", unsigned int seqNumber=0,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(), int busTrip_id=0,
			int busLine_id=0, int vehicle_id=0, unsigned int busRoute_id=0,
			Node* from=nullptr, std::string fromLocType="node", Node* to=nullptr,
			std::string toLocType="node");
	virtual ~BusTrip() {}
private:
	int busLine_id;
	int vehicle_id;
	bool direction_flag; // indicate the direction of this BusTrip( two directions only)
	BusRouteInfo* bus_RouteInfo;// route inside this BusTrip
};

}
