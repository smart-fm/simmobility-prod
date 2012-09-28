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

using std::vector;
using std::string;

namespace sim_mob {

struct BusStop_ScheduledTimes{
	explicit BusStop_ScheduledTimes(std::string scheduled_ArrivalTime, std::string scheduled_DepartureTime);
	sim_mob::DailyTime scheduled_ArrivalTime;
	sim_mob::DailyTime scheduled_DepartureTime;
};

struct BusStop_RealTimes{
	explicit BusStop_RealTimes(std::string real_ArrivalTime = "00:00:00", std::string real_DepartureTime = "00:00:00");
	sim_mob::DailyTime real_ArrivalTime;// real Arrival Time
	sim_mob::DailyTime real_DepartureTime;// real Departure Time
};

class BusStopInfo { // not clear
public:
	BusStopInfo();
	virtual ~BusStopInfo() {}
private:
	int stop_id;
	string stop_name;
	int roadsegment_id;
	BusStop_ScheduledTimes busStop_ScheduledTimes;// for each particular BusTrip with this stop_id
	BusStop_RealTimes busStop_realTimes;
};

class BusRouteInfo { // need copy constructor since BusTrip copy the BusRoute, or may need assign constructor
public:
	BusRouteInfo(unsigned int busRoute_id=0);
	BusRouteInfo(const BusRouteInfo& copyFrom); ///<Copy constructor
	virtual ~BusRouteInfo() {}

	const vector<const sim_mob::RoadSegment*>& getRoadSegments() const {
		return roadSegment_vec;
	}
	const vector<const sim_mob::BusStopInfo*>& getBusStopsInfo() const {
		return busStopInfo_vec;
	}
	const vector<const sim_mob::BusStop*>& getBusStops() const {
		return busStop_vec;
	}
	void addBusStop(const sim_mob::BusStop* aBusStop);
	void addRoadSegment(const sim_mob::RoadSegment* aRoadSegment);
	void addBusStopInfo(const sim_mob::BusStopInfo* aBusStopInfo);
private:
	unsigned int busRoute_id;
	vector<const sim_mob::RoadSegment*> roadSegment_vec;
	vector<const sim_mob::BusStopInfo*> busStopInfo_vec;
	vector<const sim_mob::BusStop*> busStop_vec;
};

class BusTrip: public sim_mob::Trip {// Can be inside the TripChain generation or BusLine stored in BusController
public:
	BusTrip(int entId=0, std::string type="BusTrip", unsigned int seqNumber=0,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(), int busTrip_id=0,
			int busLine_id=0, int vehicle_id=0, unsigned int busRoute_id=0,
			Node* from=nullptr, std::string fromLocType="node", Node* to=nullptr,
			std::string toLocType="node");
	virtual ~BusTrip() {}

	const int getBusTripID() const {
		return busTrip_id;
	}
	const int getBusLineID() const {
		return busLine_id;
	}
	const bool getDirectionFlag() const {
		return direction_flag;
	}
private:
	int busLine_id;
	int busTrip_id;
	int vehicle_id;
	bool direction_flag; // indicate the direction of this BusTrip( two directions only: fwd or rev)
	BusRouteInfo* bus_RouteInfo;// route inside this BusTrip
};


enum CONTROL_TYPE {
	NO_CONTROL, SCHEDULE_BASED, HEADWAY_BASED, EVENHEADWAY_BASED, HYBRID_BASED
};

class Busline { // busSchedule later inside PT_Schedule
public:
	Busline(int busline_id=0, std::string controlType="No_Control");
	virtual ~Busline();

	static CONTROL_TYPE getControlTypeFromString(std::string ControlType);
	const CONTROL_TYPE getControlType() const
	{
		return controlType;
	}
	const int getBusLineID() const {
		return busline_id;
	}
	void addFwdBusTrip(const sim_mob::BusTrip& aFwdBusTrip);
	void addRevBusTrip(const sim_mob::BusTrip& aRevBusTrip);
	const vector<sim_mob::BusTrip>& getFwdBusTrips() const {
		return fwdbustrip_vec;
	}
	const vector<sim_mob::BusTrip>& getRevBusTrips() const {
		return revbustrip_vec;
	}
private:
	int busline_id;
	CONTROL_TYPE controlType;
	vector<sim_mob::BusTrip> fwdbustrip_vec;
	vector<sim_mob::BusTrip> revbustrip_vec;
};

class PT_Schedule { // stored in BusController, Schedule Time Points and Real Time Points should be put separatedly
public:
	PT_Schedule();
	virtual ~PT_Schedule();

	void addBusLine(const sim_mob::Busline& aBusline);
	const vector<sim_mob::Busline>& getBuslines() const {
		return busline_vec;
	}
private:
	vector<sim_mob::Busline> busline_vec;
};

}
