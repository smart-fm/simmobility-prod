/* Copyright Singapore-MIT Alliance for Research and Technology */
/*
 * BusTrip.hpp
 *
 *  Created on: Sep 16, 2012
 *      Author: Yao Jin
 */

#pragma once

#include <vector>
#include <map>
#include <string>

#include "TripChain.hpp"
#include "geospatial/BusStop.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

using std::vector;
using std::map;
using std::string;

namespace sim_mob {

// offsetMS_From(ConfigParams::GetInstance().simStartTime))???
struct BusStop_ScheduledTimes{
	explicit BusStop_ScheduledTimes(string scheduled_ArrivalTime, string scheduled_DepartureTime);
	DailyTime scheduled_ArrivalTime;
	DailyTime scheduled_DepartureTime;
};

struct BusStop_RealTimes{
	explicit BusStop_RealTimes(string real_ArrivalTime = "00:00:00", string real_DepartureTime = "00:00:00");
	DailyTime real_ArrivalTime;// real Arrival Time
	DailyTime real_DepartureTime;// real Departure Time
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

	const vector<const RoadSegment*>& getRoadSegments() const {
		return roadSegment_vec;
	}
	const vector<const BusStopInfo*>& getBusStopsInfo() const {
		return busStopInfo_vec;
	}
	const vector<const BusStop*>& getBusStops() const {
		return busStop_vec;
	}
	void addBusStop(const BusStop* aBusStop);
	void addRoadSegment(const RoadSegment* aRoadSegment);
	void addBusStopInfo(const BusStopInfo* aBusStopInfo);
private:
	unsigned int busRoute_id;
	vector<const RoadSegment*> roadSegment_vec;
	vector<const BusStopInfo*> busStopInfo_vec;
	vector<const BusStop*> busStop_vec;
};

class BusTrip: public sim_mob::Trip {// Can be inside the TripChain generation or BusLine stored in BusController
public:
	BusTrip(int entId=0, string type="BusTrip", unsigned int seqNumber=0,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(), int busTrip_id=0,
			int busLine_id=0, int vehicle_id=0, unsigned int busRoute_id=0,
			Node* from=nullptr, string fromLocType="node", Node* to=nullptr,
			string toLocType="node");
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
	bool direction_flag; // indicate the direction of this BusTrip( two directions only: fwd-->1 or rev-->2)
	BusRouteInfo* bus_RouteInfo;// route inside this BusTrip
};


enum CONTROL_TYPE {
	NO_CONTROL, SCHEDULE_BASED, HEADWAY_BASED, EVENHEADWAY_BASED, HYBRID_BASED
};

class Busline { // busSchedule later inside PT_Schedule
public:
	Busline(int busline_id=0, string controlType="No_Control"); // default no control(only follow the schedule given)
	virtual ~Busline();

	static CONTROL_TYPE getControlTypeFromString(string ControlType);
	const CONTROL_TYPE getControlType() const
	{
		return controlType;
	}
	const int getBusLineID() const {
		return busline_id;
	}
	void addFwdBusTrip(const BusTrip& aFwdBusTrip);
	void addRevBusTrip(const BusTrip& aRevBusTrip);
	const vector<BusTrip>& getFwdBusTrips() const {
		return fwdbustrip_vec;
	}
	const vector<BusTrip>& getRevBusTrips() const {
		return revbustrip_vec;
	}
private:
	int busline_id;
	CONTROL_TYPE controlType;
	vector<BusTrip> fwdbustrip_vec;
	vector<BusTrip> revbustrip_vec;
};

class PT_Schedule { // stored in BusController, Schedule Time Points and Real Time Points should be put separatedly
public:
	PT_Schedule();
	virtual ~PT_Schedule();

	void registerBusLine(const int busline_id, const Busline* aBusline);
	const Busline* findBusline(int busline_id) const;
	const CONTROL_TYPE findBuslineControlType(int busline_id) const;
private:
	map<int, const Busline*> buslineID_busline;// need new 2 times(one for particular trip, one for backup in BusController
	map<int, const CONTROL_TYPE> buslineID_controlType;// busline--->controlType
};

}
