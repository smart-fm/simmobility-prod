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
#include "buffering/Shared.hpp"
#include "conf/simpleconf.hpp"

#include "conf/settings/DisableMPI.h"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

namespace sim_mob {
class Busline;

// offsetMS_From(ConfigParams::GetInstance().simStartTime)???
class BusStop_ScheduledTimes{
public:
	explicit BusStop_ScheduledTimes(DailyTime scheduled_ArrivalTime, DailyTime scheduled_DepartureTime);
	~BusStop_ScheduledTimes() {}
	BusStop* Scheduled_busStop; // use this to check whether the whole set is valid or not
	DailyTime scheduled_ArrivalTime;
	DailyTime scheduled_DepartureTime;
};

class BusStop_RealTimes{
public:
	explicit BusStop_RealTimes(DailyTime real_ArrivalTime = DailyTime("00:00:00"), DailyTime real_DepartureTime = DailyTime("00:00:00"));
	BusStop_RealTimes(const BusStop_RealTimes& copyFrom);
	~BusStop_RealTimes() {}
	void setReal_BusStop(const BusStop* real_busStop);
	BusStop* Real_busStop;
	DailyTime real_ArrivalTime;// real Arrival Time
	DailyTime real_DepartureTime;// real Departure Time
};

class BusRouteInfo { // need copy constructor since BusTrip copy the BusRoute, or may need assign constructor
public:
	BusRouteInfo(std::string busRoute_id="");
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
	std::string busRoute_id;
	std::vector<const RoadSegment*> roadSegment_vec;
	std::vector<const BusStop*> busStop_vec;
};

class BusTrip: public sim_mob::Trip {// Can be inside the TripChain generation or BusLine stored in BusController
public:
	//Note: I am changing the default entID value to "-1", which *should* generate Agent IDs correctly.
	BusTrip(std::string entId = "", std::string type="BusTrip", unsigned int seqNumber=0, int requestTime=-1,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(), int busTripRun_sequenceNum=0,
			Busline* busline=nullptr, int vehicle_id=0, std::string busRoute_id="",
			Node* from=nullptr, std::string fromLocType="node", Node* to=nullptr,
			std::string toLocType="node");
	virtual ~BusTrip() {}

	const int getBusTripRun_SequenceNum() const {
		return busTripRun_sequenceNum;
	}
	void setBusline(Busline* aBusline) {
		busline = aBusline;
	}
	const Busline* getBusline() const {
		return busline;
	}
	int getVehicleID() const {
		return vehicle_id;
	}
	const BusRouteInfo& getBusRouteInfo() const {
		return bus_RouteInfo;
	}

	bool setBusRouteInfo(std::vector<const RoadSegment*> roadSegment_vec, std::vector<const BusStop*> busStop_vec);
	void addBusStopScheduledTimes(const BusStop_ScheduledTimes& aBusStopScheduledTime);
	void addBusStopRealTimes(Shared<BusStop_RealTimes>* aBusStopRealTime);
	void setBusStopRealTimes(int busstopSequence_j, Shared<BusStop_RealTimes>* busStopRealTimes);
	const std::vector<BusStop_ScheduledTimes>& getBusStopScheduledTimes() const {
		return busStopScheduledTimes_vec;
	}
	const std::vector<Shared<BusStop_RealTimes>* >& getBusStopRealTimes() const {
		return busStopRealTimes_vec;
	}
	int lastVisitedStop_SequenceNumber;

private:
	int busTripRun_sequenceNum;
	int vehicle_id;
	Busline* busline; // indicate the busline pointer. save when assigned all bustrips.
	BusRouteInfo bus_RouteInfo;// route inside this BusTrip, just some roadSegments and BusStops

	//each bustrip holds last sequence number.default -1 no busstop visited yet
	std::vector<BusStop_ScheduledTimes> busStopScheduledTimes_vec;// can be different for different pair<busLine_id,busTripRun_sequenceNum>
	std::vector<Shared<BusStop_RealTimes>* > busStopRealTimes_vec;// can be different for different pair<busLine_id,busTripRun_sequenceNum>
};


enum CONTROL_TYPE {
	NO_CONTROL, SCHEDULE_BASED, HEADWAY_BASED, EVENHEADWAY_BASED, HYBRID_BASED
};

class Frequency_Busline {
public:
	Frequency_Busline(DailyTime start_Time=DailyTime("00:00:00"), DailyTime end_Time=DailyTime("00:00:00"), int headway=0);
	DailyTime start_Time;// DailyTime start time
	DailyTime end_Time;// DailyTime end time
	int headway;// sec
};

class Busline { // busSchedule later inside PT_Schedule
public:
	Busline(std::string busline_id="", std::string controlType="no_control"); // default no control(only follow the schedule given)
	virtual ~Busline();

	static CONTROL_TYPE getControlTypeFromString(std::string ControlType);
	const CONTROL_TYPE getControlType() const
	{
		return controlType;
	}
	const std::string& getBusLineID() const {
		return busline_id;
	}
	const int getControl_TimePointNum0() const {
		return control_TimePointNum0;
	}
	const int getControl_TimePointNum1() const {
		return control_TimePointNum1;
	}
	void addBusTrip(BusTrip& aBusTrip);
	void addFrequencyBusline(const Frequency_Busline& aFrequencyBusline);
	const std::vector<BusTrip>& queryBusTrips() const {
		return busTrip_vec;
	}
	const std::vector<Frequency_Busline>& query_Frequency_Busline() const {
		return frequency_busline;
	}
	void resetBusTrip_StopRealTimes(int trip_k, int busstopSequence_j, Shared<BusStop_RealTimes>* busStopRealTimes);// mainly for realTimes
private:
	std::string busline_id;
	CONTROL_TYPE controlType;
	std::vector<Frequency_Busline> frequency_busline; // provide different headways according to the offset from simulation for each busline
	std::vector<BusTrip> busTrip_vec;// constructed based on MSOffset_headway
	int control_TimePointNum0; // now only one time point(hardcoded), later extend to the vector<control_TimePoint>
	int control_TimePointNum1; // now another time point (hardcoded)
};

class PT_Schedule { // stored in BusController, Schedule Time Points and Real Time Points should be put separatedly
public:
	PT_Schedule();
	virtual ~PT_Schedule();

	void registerBusLine(const std::string busline_id, Busline* aBusline);
	void registerControlType(const std::string busline_id, const CONTROL_TYPE aControlType);
	Busline* findBusline(const std::string& busline_id);
	CONTROL_TYPE findBuslineControlType(const std::string& busline_id);
	std::map<std::string, Busline*>& get_busLines() { return buslineID_busline; }
private:
	std::map<std::string, Busline*> buslineID_busline;// need new 2 times(one for particular trip, one for backup in BusController
	std::map<std::string, CONTROL_TYPE> buslineID_controlType;// busline--->controlType
};

}
