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

// offsetMS_From(ConfigParams::GetInstance().simStartTime))???
class BusStop_ScheduledTimes{
public:
	explicit BusStop_ScheduledTimes(unsigned int scheduled_ArrivalTime, unsigned int scheduled_DepartureTime);
	~BusStop_ScheduledTimes() {}
	int stop_id;
	unsigned int scheduled_ArrivalTime;
	unsigned int scheduled_DepartureTime;
};

class BusStop_RealTimes{
public:
	explicit BusStop_RealTimes(unsigned int real_ArrivalTime = 0, unsigned int real_DepartureTime = 0);
	~BusStop_RealTimes() {}
	int stop_id;
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
	const vector<const BusStop_ScheduledTimes*>& getBusStopScheduledTimes() const {
		return busStopScheduledTimes_vec;
	}
	const vector <Shared<BusStop_RealTimes>* >& getBusStopRealTimes() const {
		return busStopRealTimes_vec;
	}
	void addBusStop(const BusStop* aBusStop);
	void addRoadSegment(const RoadSegment* aRoadSegment);
	void addBusStopScheduledTimes(const BusStop_ScheduledTimes* aBusStopScheduledTime);
	void addBusStopRealTimes(Shared<BusStop_RealTimes>* aBusStopRealTime);
	void setBusStopRealTimes(int busstopSequence_j, BusStop_RealTimes& busStopRealTimes);
private:
	unsigned int busRoute_id;
	vector<const RoadSegment*> roadSegment_vec;
	vector<const BusStop*> busStop_vec;
	vector<const BusStop_ScheduledTimes*> busStopScheduledTimes_vec;
	vector <Shared<BusStop_RealTimes>* > busStopRealTimes_vec;
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
	const BusRouteInfo* queryBusRouteInfo() const {
		return bus_RouteInfo;
	}
	BusRouteInfo* getBusRouteInfo() const {
		return bus_RouteInfo;
	}
private:
	int busLine_id;
	int busTripRun_sequenceNum;
	int vehicle_id;
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
//	void addFwdBusTrip(const BusTrip& aFwdBusTrip);
//	void addRevBusTrip(const BusTrip& aRevBusTrip);
//	const vector<BusTrip>& getFwdBusTrips() const {
//		return fwdbustrip_vec;
//	}
//	const vector<BusTrip>& getRevBusTrips() const {
//		return revbustrip_vec;
//	}
	void addBusTrip(const BusTrip& aBusTrip);
	const vector<BusTrip>& getBusTrips() const {
		return busTrip_vec;
	}
private:
	int busline_id;
	CONTROL_TYPE controlType;
	vector<BusTrip> busTrip_vec;
	//vector<BusTrip> fwdbustrip_vec;
	//vector<BusTrip> revbustrip_vec;
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
