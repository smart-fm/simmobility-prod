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
	explicit BusStop_RealTimes(DailyTime real_ArrivalTime = DailyTime("00:00:00"), DailyTime real_DepartureTime = DailyTime("00:00:00"));
	~BusStop_RealTimes() {}
	BusStop* Real_busStop;
	DailyTime real_ArrivalTime;// real Arrival Time
	DailyTime real_DepartureTime;// real Departure Time
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

	const std::vector<const RoadSegment*>& getRoadSegments() const {
		return roadSegment_vec;
	}
	const std::vector<const BusStop*>& getBusStops() const {
		return busStop_vec;
	}

	void addBusStop(const BusStop* aBusStop);
	void addRoadSegment(const RoadSegment* aRoadSegment);

private:
	unsigned int busRoute_id;
	std::vector<const RoadSegment*> roadSegment_vec;
	std::vector<const BusStop*> busStop_vec;
};

class BusTrip: public sim_mob::Trip {// Can be inside the TripChain generation or BusLine stored in BusController
public:
	BusTrip(int entId=0, std::string type="BusTrip", unsigned int seqNumber=0,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(), int busTripRun_sequenceNum=0,
			int busLine_id=0, int vehicle_id=0, unsigned int busRoute_id=0,
			Node* from=nullptr, std::string fromLocType="node", Node* to=nullptr,
			std::string toLocType="node");
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
	const std::vector<BusStop_ScheduledTimes>& getBusStopScheduledTimes() const {
		return busStopScheduledTimes_vec;
	}
	const std::vector<Shared<BusStop_RealTimes>* >& getBusStopRealTimes() const {
		return busStopRealTimes_vec;
	}
private:
	int busLine_id;
	int busTripRun_sequenceNum;
	int vehicle_id;
	BusRouteInfo bus_RouteInfo;// route inside this BusTrip, just some roadSegments and BusStops

	std::vector<BusStop_ScheduledTimes> busStopScheduledTimes_vec;// can be different for different pair<busLine_id,busTripRun_sequenceNum>
	std::vector<Shared<BusStop_RealTimes>* > busStopRealTimes_vec;// can be different for different pair<busLine_id,busTripRun_sequenceNum>
};


enum CONTROL_TYPE {
	NO_CONTROL, SCHEDULE_BASED, HEADWAY_BASED, EVENHEADWAY_BASED, HYBRID_BASED
};

class Frequency_Busline {
public:
	unsigned int start_Time;// MS offset from simulation start time
	unsigned int end_Time;// MS offset from simulation start time
	int headway;// sec
};

class Busline { // busSchedule later inside PT_Schedule
public:
	Busline(std::string busline_id="", std::string controlType="No_Control"); // default no control(only follow the schedule given)
	virtual ~Busline();

	static CONTROL_TYPE getControlTypeFromString(std::string ControlType);
	const CONTROL_TYPE getControlType() const
	{
		return controlType;
	}
	const std::string& getBusLineID() const {
		return busline_id;
	}
	void addBusTrip(BusTrip& aBusTrip);
	const std::vector<BusTrip>& queryBusTrips() const {
		return busTrip_vec;
	}
	const std::vector<Frequency_Busline>& query_Frequency_Busline() const {
		return frequency_busline;
	}
	void resetBusTrip_StopRealTimes(int trip_k, int busstopSequence_j, BusStop_RealTimes& busStopRealTimes);// mainly for realTimes
private:
	std::string busline_id;
	CONTROL_TYPE controlType;
	std::vector<Frequency_Busline> frequency_busline; // provide different headways according to the offset from simulation for each busline
	std::vector<BusTrip> busTrip_vec;// constructed based on MSOffset_headway
};

class PT_Schedule { // stored in BusController, Schedule Time Points and Real Time Points should be put separatedly
public:
	PT_Schedule();
	virtual ~PT_Schedule();

	void registerBusLine(const std::string busline_id, Busline* aBusline);
	Busline* findBusline(const std::string& busline_id);
	const CONTROL_TYPE findBuslineControlType(const std::string& busline_id) const;
private:
	std::map<std::string, Busline*> buslineID_busline;// need new 2 times(one for particular trip, one for backup in BusController
	std::map<std::string, const CONTROL_TYPE> buslineID_controlType;// busline--->controlType
};

}
