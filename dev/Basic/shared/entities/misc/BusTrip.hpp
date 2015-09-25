//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <map>
#include <string>


#include "buffering/Shared.hpp"
#include "conf/settings/DisableMPI.h"
#include "entities/misc/TripChain.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

namespace sim_mob {

class BusLine;
class BusStopScheduledTimes {
public:
	explicit BusStopScheduledTimes(DailyTime arrivalTime,
			DailyTime departureTime);
	~BusStopScheduledTimes() {
	}
	/**
	 * reference bus stop
	 */
	BusStop* busStop;
	/**
	 * scheduled Arrival Time
	 */
	DailyTime arrivalTime;
	/**
	 * scheduled Departure Time
	 */
	DailyTime departureTime;
};


class BusStopRealTimes {
public:
	explicit BusStopRealTimes(
			DailyTime arrivalTime = DailyTime("00:00:00"),
			DailyTime departureTime = DailyTime("00:00:00"));
	BusStopRealTimes(const BusStopRealTimes& copyFrom);
	~BusStopRealTimes() {
	}
	void setRealBusStop(const BusStop* real_busStop);
	/**
	 * reference bus stop
	 */
	BusStop* busStop;
	/**
	 * real Arrival Time
	 */
	DailyTime arrivalTime;
	/**
	 * real Departure Time
	 */
	DailyTime departureTime;
};


class BusRouteInfo {
public:
	BusRouteInfo(std::string busRoute_id = "");
	BusRouteInfo(const BusRouteInfo& copyFrom);
	virtual ~BusRouteInfo() {
	}

	const std::string& getBusRouteId() const {
		return busRouteId;
	}
	const std::vector<const RoadSegment*>& getRoadSegments() const {
		return roadSegmentList;
	}
	const std::vector<const BusStop*>& getBusStops() const {
		return busStopList;
	}

	void addBusStop(const BusStop* stop);
	void addRoadSegment(const RoadSegment* segment);

protected:
	/**
	 * bus route id
	 */
	std::string busRouteId;
	/**
	 * segment list for this bus route
	 */
	std::vector<const RoadSegment*> roadSegmentList;
	/**
	 * stop list for this bus route
	 */
	std::vector<const BusStop*> busStopList;
};


class BusTrip: public sim_mob::Trip {
public:
	BusTrip(std::string entId = "", std::string type = "BusTrip",
			unsigned int seqNumber = 0, int requestTime = -1,
			DailyTime start = DailyTime(), DailyTime end = DailyTime(),
			int totalSequenceNum = 0, BusLine* busLine = nullptr,
			int vehicleId = 0, std::string busRouteId = "",
			Node* from = nullptr, std::string fromLocType = "node",
			Node* to = nullptr, std::string toLocType = "node");
	virtual ~BusTrip() {
	}

	const int getTotalSequenceNum() const {
		return totalSequenceNum;
	}
	const int getBusTripStopIndex(const BusStop* stop) const;

	void setBusline(BusLine* line) {
		busLine = line;
	}
	const BusLine* getBusLine() const {
		return busLine;
	}
	int getVehicleID() const {
		return vehicleId;
	}
	const BusRouteInfo& getBusRouteInfo() const {
		return busRouteInfo;
	}

	virtual const std::string getMode() const {
		return "Bus";
	}

	bool setBusRouteInfo(std::vector<const RoadSegment*> roadSegments,	std::vector<const BusStop*> busStops);

	void addBusStopScheduledTimes(const BusStopScheduledTimes& aBusStopScheduledTime);

	void addBusStopRealTimes(Shared<BusStopRealTimes>* aBusStopRealTime);

	void setBusStopRealTimes(int stopSequence, Shared<BusStopRealTimes>* busStopRealTimes);

	const std::vector<BusStopScheduledTimes>& getBusStopScheduledTimes() const {
		return busStopScheduledTimes;
	}

	const std::vector<Shared<BusStopRealTimes>*>& getBusStopRealTimes() const {
		return busStopRealTimes;
	}
	void safeDeleteBusStopRealTimes() {
		clear_delete_vector(busStopRealTimes);
	}

	int getLastStopSequenceNumber() const {
		return lastVisitedStopSequenceNumber;
	}

	void setLastStopSequenceNumber(int seq) {
		lastVisitedStopSequenceNumber = seq;
	}

private:
	int lastVisitedStopSequenceNumber;
	/**
	 * the sequence number for all the trips
	 */
	int totalSequenceNum;
	/**
	 * vehicle id associated with this bus trip
	 */
	int vehicleId;
	/**
	 * indicate the bus line pointer. save when assigned all bus trips.
	 */
	BusLine* busLine;
	/**
	 * route inside this BusTrip, just some roadSegments and BusStops
	 */
	BusRouteInfo busRouteInfo;

	/**
	 * scheduled time at each bus stop
	 */
	std::vector<BusStopScheduledTimes> busStopScheduledTimes;
	/**
	 * real arrival time at each bus stop
	 */
	std::vector<Shared<BusStopRealTimes>*> busStopRealTimes;
};


enum ControlTypes {
	NO_CONTROL, SCHEDULE_BASED, HEADWAY_BASED, EVENHEADWAY_BASED, HYBRID_BASED
};

class FrequencyBusLine {
public:
	FrequencyBusLine(DailyTime startTime = DailyTime("00:00:00"),
			DailyTime endTime = DailyTime("00:00:00"), int headway = 0);
	/**
	 * starting time for first dispatched bus
	 */
	DailyTime startTime;
	/**
	 * ending time for last dispatched bus
	 */
	DailyTime endTime;
	/**
	 * dispatching frequency
	 */
	int headwaySec;
};

class BusLine {
public:
	BusLine(std::string busLine = "", std::string controlType = "no_control");
	virtual ~BusLine();

	static ControlTypes getControlTypeFromString(std::string controlType);
	const ControlTypes getControlType() const {
		return controlType;
	}
	const std::string& getBusLineID() const {
		return busLineId;
	}
	const int getControlTimePointNum0() const {
		return controlTimePointNum0;
	}
	const int getControlTimePointNum1() const {
		return controlTimePointNum1;
	}
	const int getControlTimePointNum2() const {
		return controlTimePointNum2;
	}
	const int getControlTimePointNum3() const {
		return controlTimePointNum3;
	}
	void addBusTrip(BusTrip& aBusTrip);
	void addFrequencyBusLine(const FrequencyBusLine& aFrequencyBusline);
	const std::vector<BusTrip>& queryBusTrips() const {
		return busTrips;
	}
	const std::vector<FrequencyBusLine>& queryFrequencyBusline() const {
		return frequencyBusLine;
	}

	void resetBusTripStopRealTimes(int trip, int stopSequence,
			Shared<BusStopRealTimes>* busStopRealTimes);

private:
	/**
	 * bus line id
	 */
	std::string busLineId;
	/**
	 * control type for holding strategy
	 */
	ControlTypes controlType;
	/**
	 * provide different headways according to the offset from simulation for each busline
	 */
	std::vector<FrequencyBusLine> frequencyBusLine;
	/**
	 * constructed based on MSOffset_headway
	 */
	std::vector<BusTrip> busTrips;
	/**
	 * now only one time point
	 */
	int controlTimePointNum0;
	int controlTimePointNum1;
	int controlTimePointNum2;
	int controlTimePointNum3;
};
/**
 * stored in BusController, Schedule Time Points and Real Time Points should be put seperatedly
 */
class PT_Schedule {
public:
	PT_Schedule();
	virtual ~PT_Schedule();

	/**
	 * register a new bus line
	 */
	void registerBusLine(const std::string buslineId, BusLine* line);
	/**
	 * register a new control type for holding strategy
	 */
	void registerControlType(const std::string buslineId, const ControlTypes controlType);
	/**
	 * find a bus line from registered set
	 */
	BusLine* findBusLine(const std::string& buslineId);
	/**
	 * find a control type from registered control set
	 */
	ControlTypes findBusLineControlType(const std::string& buslineId);
	/**
	 * get bus line map from id to bus line
	 */
	std::map<std::string, BusLine*>& getBusLines() { return buslineIdBuslineMap; }
private:
	/**
	 * the data map from line id to bus line
	 */
	std::map<std::string, BusLine*> buslineIdBuslineMap;
	/**
	 * the data map from control type to control item
	 */
	std::map<std::string, ControlTypes> buslineIdControlTypeMap;
};

}
