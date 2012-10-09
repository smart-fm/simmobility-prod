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
#include "buffering/Shared.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

using std::vector;
using std::map;
using std::string;

namespace sim_mob {

// offsetMS_From(ConfigParams::GetInstance().simStartTime)???
class BusStop_ScheduledTimes{
public:
	explicit BusStop_ScheduledTimes(DailyTime scheduled_ArrivalTime, DailyTime scheduled_DepartureTime);
	~BusStop_ScheduledTimes() {}
	BusStop* Scheduled_busStop;
	DailyTime scheduled_ArrivalTime;
	DailyTime scheduled_DepartureTime;
};

class BusStop_RealTimes{
public:
	explicit BusStop_RealTimes(unsigned int real_ArrivalTime = 0, unsigned int real_DepartureTime = 0);
	~BusStop_RealTimes() {}
	BusStop* Real_busStop;
	unsigned int real_ArrivalTime;// real Arrival Time
	unsigned int real_DepartureTime;// real Departure Time
};

//class BusStopInfo { // not clear
//public:
//	BusStopInfo();
//	virtual ~BusStopInfo() {}
//
//	const int getBusStopID() const {
//		return stop_id;
//	}
//	const int getRoadSegmentID() const {
//		return roadsegment_id;
//	}
//	Shared<BusStop_ScheduledTimes> busStop_ScheduledTimes;// for each particular BusTrip with this stop_id
//	mutable Shared<BusStop_RealTimes> busStop_realTimes;
//private:
//	int stop_id;
//	string stop_name;
//	int roadsegment_id;
//};

class BusRouteInfo { // need copy constructor since BusTrip copy the BusRoute, or may need assign constructor
public:
	BusRouteInfo(unsigned int busRoute_id=0);
	BusRouteInfo(const BusRouteInfo& copyFrom); ///<Copy constructor
	virtual ~BusRouteInfo() {}

	const vector<const RoadSegment*>& getRoadSegments() const {
		return roadSegment_vec;
	}
	const vector<const BusStop*>& getBusStops() const {
		return busStop_vec;
	}

	void addBusStop(const BusStop* aBusStop);
	void addRoadSegment(const RoadSegment* aRoadSegment);

private:
	unsigned int busRoute_id;
	vector<const RoadSegment*> roadSegment_vec;
	vector<const BusStop*> busStop_vec;
};

class BusTrip: public sim_mob::Trip {// Can be inside the TripChain generation or BusLine stored in BusController
public:
	BusTrip(int entId=0, string type="BusTrip", unsigned int seqNumber=0,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(), int busTripRun_sequenceNum=0,
			int busLine_id=0, int vehicle_id=0, unsigned int busRoute_id=0,
			Node* from=nullptr, string fromLocType="node", Node* to=nullptr,
			string toLocType="node");
	virtual ~BusTrip() {}

	const int getBusTripRun_SequenceNum() const {
		return busTripRun_sequenceNum;
	}
	const int getBusLineID() const {
		return busLine_id;
	}
	const BusRouteInfo& getBusRouteInfo() const {
		return bus_RouteInfo;
	}
//	BusRouteInfo* getBusRouteInfo() const {
//		return bus_RouteInfo;
//	}
	void addBusStopScheduledTimes(const BusStop_ScheduledTimes& aBusStopScheduledTime);
	void addBusStopRealTimes(Shared<BusStop_RealTimes>* aBusStopRealTime);
	void setBusStopRealTimes(int busstopSequence_j, BusStop_RealTimes& busStopRealTimes);
	const vector<BusStop_ScheduledTimes>& getBusStopScheduledTimes() const {
		return busStopScheduledTimes_vec;
	}
	const vector <Shared<BusStop_RealTimes>* >& getBusStopRealTimes() const {
		return busStopRealTimes_vec;
	}
private:
	int busLine_id;
	int busTripRun_sequenceNum;
	int vehicle_id;
	BusRouteInfo bus_RouteInfo;// route inside this BusTrip, just some roadSegments and BusStops

	vector<BusStop_ScheduledTimes> busStopScheduledTimes_vec;// can be different for different pair<busLine_id,busTripRun_sequenceNum>
	vector <Shared<BusStop_RealTimes>* > busStopRealTimes_vec;// can be different for different pair<busLine_id,busTripRun_sequenceNum>
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
	void addBusTrip(BusTrip& aBusTrip);
	const vector<BusTrip>& queryBusTrips() const {
		return busTrip_vec;
	}
	void resetBusTrip_StopRealTimes(int trip_k, int busstopSequence_j, BusStop_RealTimes& busStopRealTimes);// mainly for realTimes
private:
	int busline_id;
	CONTROL_TYPE controlType;
	vector<BusTrip> busTrip_vec;
};

class PT_Schedule { // stored in BusController, Schedule Time Points and Real Time Points should be put separatedly
public:
	PT_Schedule();
	virtual ~PT_Schedule();

	void registerBusLine(const int busline_id, Busline* aBusline);
	Busline* findBusline(int busline_id);
	const CONTROL_TYPE findBuslineControlType(int busline_id) const;
private:
	map<int, Busline*> buslineID_busline;// need new 2 times(one for particular trip, one for backup in BusController
	map<int, const CONTROL_TYPE> buslineID_controlType;// busline--->controlType
};

}
