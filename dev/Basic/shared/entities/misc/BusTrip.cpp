/* Copyright Singapore-MIT Alliance for Research and Technology */
/*
 * BusTrip.cpp
 *
 *  Created on: Sep 16, 2012
 *      Author: Yao Jin
 */
#include "BusTrip.hpp"

using namespace sim_mob;

sim_mob::BusStop_Times::BusStop_Times(std::string scheduled_ArrivalTime, std::string scheduled_DepartureTime,
		std::string real_ArrivalTime, std::string real_DepartureTime)
: scheduled_ArrivalTime(DailyTime(scheduled_ArrivalTime)), scheduled_DepartureTime(DailyTime(scheduled_DepartureTime)),
  real_ArrivalTime(DailyTime(real_ArrivalTime)), real_DepartureTime(DailyTime(real_DepartureTime))
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
busLine_id(busLine_id), vehicle_id(vehicle_id), bus_RouteInfo(&BusRouteInfo(busRoute_id))
{

}
