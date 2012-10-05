/* Copyright Singapore-MIT Alliance for Research and Technology */
/*
 * BusTrip.cpp
 *
 *  Created on: Sep 16, 2012
 *      Author: Yao Jin
 */
#include "BusTrip.hpp"

using namespace sim_mob;

sim_mob::BusStop_ScheduledTimes::BusStop_ScheduledTimes(unsigned int scheduled_ArrivalTime, unsigned int scheduled_DepartureTime)
: scheduled_ArrivalTime(scheduled_ArrivalTime), scheduled_DepartureTime(scheduled_DepartureTime)
{

}

sim_mob::BusStop_RealTimes::BusStop_RealTimes(unsigned int real_ArrivalTime, unsigned int real_DepartureTime)
: real_ArrivalTime(real_ArrivalTime), real_DepartureTime(real_DepartureTime)
{

}

//BusStop
//BusRoute
sim_mob::BusRouteInfo::BusRouteInfo(unsigned int busRoute_id)
: busRoute_id(busRoute_id)
{

}

sim_mob::BusRouteInfo::BusRouteInfo(const BusRouteInfo& copyFrom)
: busRoute_id(copyFrom.busRoute_id), roadSegment_vec(copyFrom.roadSegment_vec)
, busStop_vec(copyFrom.busStop_vec)
{

}

void sim_mob::BusRouteInfo::addBusStop(const BusStop* aBusStop)
{
	busStop_vec.push_back(aBusStop);
}

void sim_mob::BusRouteInfo::addRoadSegment(const RoadSegment* aRoadSegment)
{
	roadSegment_vec.push_back(aRoadSegment);
}

sim_mob::BusTrip::BusTrip(int entId, string type, unsigned int seqNumber,
		DailyTime start, DailyTime end, int busTripRun_sequenceNum, int busLine_id, int vehicle_id,
		unsigned int busRoute_id, Node* from, string fromLocType, Node* to,
		string toLocType)
: Trip(entId, type, seqNumber, start, end, busTripRun_sequenceNum,from, fromLocType, to, toLocType),
busLine_id(busLine_id), busTripRun_sequenceNum(busTripRun_sequenceNum), vehicle_id(vehicle_id), bus_RouteInfo(&BusRouteInfo(busRoute_id))
{

}

void sim_mob::BusTrip::addBusStopScheduledTimes(const BusStop_ScheduledTimes* aBusStopScheduledTime)
{
	busStopScheduledTimes_vec.push_back(aBusStopScheduledTime);
}

void sim_mob::BusTrip::addBusStopRealTimes(Shared<BusStop_RealTimes>* aBusStopRealTime)
{
	busStopRealTimes_vec.push_back(aBusStopRealTime);
}

void sim_mob::BusTrip::setBusStopRealTimes(int busstopSequence_j, BusStop_RealTimes& busStopRealTimes)
{
	busStopRealTimes_vec[busstopSequence_j]->set(busStopRealTimes);
}

sim_mob::Busline::Busline(int busline_id, string controlType)
: controlType(getControlTypeFromString(controlType))
{

}

sim_mob::Busline::~Busline()
{

}

CONTROL_TYPE sim_mob::Busline::getControlTypeFromString(string ControlType)
{
	ControlType.erase(remove_if(ControlType.begin(), ControlType.end(), isspace),
			ControlType.end());
	if (ControlType == "no_control") {
		return NO_CONTROL;
	} else if (ControlType == "schedule_based") {
		return SCHEDULE_BASED;
	} else if (ControlType == "headway_based") {
		return HEADWAY_BASED;
	} else if (ControlType == "evenheadway_based") {
		return EVENHEADWAY_BASED;
	} else if (ControlType == "hybrid_based") {
		return HYBRID_BASED;
	} else {
		throw std::runtime_error("Unexpected control type.");
	}
}

void sim_mob::Busline::addBusTrip(const BusTrip* aBusTrip)
{
	busTrip_vec.push_back(aBusTrip);
}

void sim_mob::Busline::resetBusTrip_StopRealTimes(int trip_k, int busstopSequence_j, BusStop_RealTimes& busStopRealTimes) const
{
	if(!busTrip_vec.empty()) {
		BusTrip* busTripK = const_cast<BusTrip*>(busTrip_vec[trip_k]);
		busTripK->setBusStopRealTimes(busstopSequence_j, busStopRealTimes);
	}
}

sim_mob::PT_Schedule::PT_Schedule()
{

}

sim_mob::PT_Schedule::~PT_Schedule()
{

}

void sim_mob::PT_Schedule::registerBusLine(const int busline_id, const Busline* aBusline)
{
	if (buslineID_busline.count(busline_id)>0) {
		throw std::runtime_error("Duplicate buslineid.");
	}

	buslineID_busline[busline_id] = aBusline;
}

const Busline* sim_mob::PT_Schedule::findBusline(int busline_id) const
{
	map<int ,const Busline*>::const_iterator it;
	it = buslineID_busline.find(busline_id);
	if (it!=buslineID_busline.end()) {
		return it->second;
	}
	return nullptr;
}

const CONTROL_TYPE sim_mob::PT_Schedule::findBuslineControlType(int busline_id) const
{
	map<int, const CONTROL_TYPE>::const_iterator it;
	it = buslineID_controlType.find(busline_id);
	if (it!=buslineID_controlType.end()) {
		return it->second;
	}
	return NO_CONTROL;
}
