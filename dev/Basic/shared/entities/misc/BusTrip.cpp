/* Copyright Singapore-MIT Alliance for Research and Technology */
/*
 * BusTrip.cpp
 *
 *  Created on: Sep 16, 2012
 *      Author: Yao Jin
 */
#include "BusTrip.hpp"

using namespace sim_mob;

sim_mob::BusStop_ScheduledTimes::BusStop_ScheduledTimes(DailyTime scheduled_ArrivalTime, DailyTime scheduled_DepartureTime)
: scheduled_ArrivalTime(scheduled_ArrivalTime), scheduled_DepartureTime(scheduled_DepartureTime)
{

}

sim_mob::BusStop_RealTimes::BusStop_RealTimes(DailyTime real_ArrivalTime, DailyTime real_DepartureTime)
: real_ArrivalTime(real_ArrivalTime), real_DepartureTime(real_DepartureTime)
{

}

//BusStop
//BusRoute
sim_mob::BusRouteInfo::BusRouteInfo(std::string busRoute_id)
: busRoute_id(busRoute_id)
{
	roadSegment_vec.clear();
	busStop_vec.clear();
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

sim_mob::BusTrip::BusTrip(int entId, std::string type, unsigned int seqNumber,
		DailyTime start, DailyTime end, int busTripRun_sequenceNum, std::string busLine_id, int vehicle_id,
		std::string busRoute_id, Node* from, std::string fromLocType, Node* to,
		std::string toLocType)
: Trip(entId, type, seqNumber, start, end, busTripRun_sequenceNum,from, fromLocType, to, toLocType),
busLine_id(busLine_id), busTripRun_sequenceNum(busTripRun_sequenceNum), vehicle_id(vehicle_id), bus_RouteInfo(BusRouteInfo(busRoute_id))
{

}

void sim_mob::BusTrip::addBusStopScheduledTimes(const BusStop_ScheduledTimes& aBusStopScheduledTime)
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

void sim_mob::BusTrip::setBusRouteInfo(std::vector<const RoadSegment*>& roadSegment_vec)
{
	if(roadSegment_vec.empty()) {
		std::cout << "Error: no roadSegments!!!" << std::endl;
		return;
	}
	for(int i = 0; i < roadSegment_vec.size(); i++) {
		bus_RouteInfo.addRoadSegment(roadSegment_vec[i]);
	}
	// later add argument std::vector<const BusStop*>& busStop_vec
}

sim_mob::Frequency_Busline::Frequency_Busline(DailyTime start_Time, DailyTime end_Time, int headway)
: start_Time(start_Time), end_Time(end_Time), headway(headway)
{

}

sim_mob::Busline::Busline(std::string busline_id, std::string controlType)
: busline_id(busline_id), controlType(getControlTypeFromString(controlType))
{

}

sim_mob::Busline::~Busline()
{

}

CONTROL_TYPE sim_mob::Busline::getControlTypeFromString(std::string ControlType)
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

void sim_mob::Busline::addBusTrip(BusTrip& aBusTrip)
{
	busTrip_vec.push_back(aBusTrip);
}

void sim_mob::Busline::addFrequencyBusline(Frequency_Busline& aFrequencyBusline)
{
	frequency_busline.push_back(aFrequencyBusline);
}

void sim_mob::Busline::resetBusTrip_StopRealTimes(int trip_k, int busstopSequence_j, BusStop_RealTimes& busStopRealTimes)
{
	if(!busTrip_vec.empty()) {
		//BusTrip& busTripK = busTrip_vec[trip_k];
		busTrip_vec[trip_k].setBusStopRealTimes(busstopSequence_j, busStopRealTimes);
	}
}

sim_mob::PT_Schedule::PT_Schedule()
{

}

sim_mob::PT_Schedule::~PT_Schedule()
{

}

void sim_mob::PT_Schedule::registerBusLine(const std::string busline_id, Busline* aBusline)
{
	if (buslineID_busline.count(busline_id)>0) {
		throw std::runtime_error("Duplicate buslineid.");
	}

	buslineID_busline[busline_id] = aBusline;
}

Busline* sim_mob::PT_Schedule::findBusline(const std::string& busline_id)
{
	std::map<std::string, Busline*>::const_iterator it;
	it = buslineID_busline.find(busline_id);
	if (it!=buslineID_busline.end()) {
		return it->second;
	}
	return nullptr;
}

const CONTROL_TYPE sim_mob::PT_Schedule::findBuslineControlType(const std::string& busline_id) const
{
	std::map<std::string, const CONTROL_TYPE>::const_iterator it;
	it = buslineID_controlType.find(busline_id);
	if (it!=buslineID_controlType.end()) {
		return it->second;
	}
	return NO_CONTROL;
}
