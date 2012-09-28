/* Copyright Singapore-MIT Alliance for Research and Technology */
/*
 * BusTrip.cpp
 *
 *  Created on: Sep 16, 2012
 *      Author: Yao Jin
 */
#include "BusTrip.hpp"

using namespace sim_mob;

sim_mob::BusStop_ScheduledTimes::BusStop_ScheduledTimes(std::string scheduled_ArrivalTime, std::string scheduled_DepartureTime)
: scheduled_ArrivalTime(DailyTime(scheduled_ArrivalTime)), scheduled_DepartureTime(DailyTime(scheduled_DepartureTime))
{

}

sim_mob::BusStop_RealTimes::BusStop_RealTimes(std::string real_ArrivalTime, std::string real_DepartureTime)
: real_ArrivalTime(DailyTime(real_ArrivalTime)), real_DepartureTime(DailyTime(real_DepartureTime))
{

}

//BusStop
//BusRoute
sim_mob::BusRouteInfo::BusRouteInfo(unsigned int busRoute_id)
: busRoute_id(busRoute_id)
{

}

sim_mob::BusRouteInfo::BusRouteInfo(const BusRouteInfo& copyFrom)
: busRoute_id(copyFrom.busRoute_id), roadSegment_vec(copyFrom.roadSegment_vec), busStopInfo_vec(copyFrom.busStopInfo_vec)
{

}

void sim_mob::BusRouteInfo::addBusStop(const sim_mob::BusStop* aBusStop)
{
	busStop_vec.push_back(aBusStop);
}

void sim_mob::BusRouteInfo::addRoadSegment(const sim_mob::RoadSegment* aRoadSegment)
{
	roadSegment_vec.push_back(aRoadSegment);
}

void sim_mob::BusRouteInfo::addBusStopInfo(const sim_mob::BusStopInfo* aBusStopInfo)
{
	busStopInfo_vec.push_back(aBusStopInfo);
}

sim_mob::BusTrip::BusTrip(int entId, std::string type, unsigned int seqNumber,
		DailyTime start, DailyTime end, int busTrip_id, int busLine_id, int vehicle_id,
		unsigned int busRoute_id, Node* from, std::string fromLocType, Node* to,
		std::string toLocType)
: Trip(entId, type, seqNumber, start, end, busTrip_id,from, fromLocType, to, toLocType),
busLine_id(busLine_id), busTrip_id(busTrip_id), vehicle_id(vehicle_id), bus_RouteInfo(&BusRouteInfo(busRoute_id))
{

}

sim_mob::Busline::Busline(int busline_id, std::string controlType)
: controlType(getControlTypeFromString(controlType))
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

void sim_mob::Busline::addFwdBusTrip(const sim_mob::BusTrip& aFwdBusTrip)
{
	fwdbustrip_vec.push_back(aFwdBusTrip);
}

void sim_mob::Busline::addRevBusTrip(const sim_mob::BusTrip& aRevBusTrip)
{
	revbustrip_vec.push_back(aRevBusTrip);
}

sim_mob::PT_Schedule::PT_Schedule()
{

}

sim_mob::PT_Schedule::~PT_Schedule()
{

}

void sim_mob::PT_Schedule::addBusLine(const sim_mob::Busline& aBusline)
{
	busline_vec.push_back(aBusline);
}
