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
	Real_busStop = nullptr;
}

sim_mob::BusStop_RealTimes::BusStop_RealTimes(const BusStop_RealTimes& copyFrom)
: real_ArrivalTime(copyFrom.real_ArrivalTime), real_DepartureTime(copyFrom.real_DepartureTime)
, Real_busStop(copyFrom.Real_busStop)
{

}

void sim_mob::BusStop_RealTimes::setReal_BusStop(const BusStop* real_busStop)
{
	Real_busStop = const_cast<BusStop*>(real_busStop);
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
		DailyTime start, DailyTime end, int busTripRun_sequenceNum,
		Busline* busline, int vehicle_id, std::string busRoute_id, Node* from,
		std::string fromLocType, Node* to, std::string toLocType)
: Trip(entId, type, seqNumber, start, end, busTripRun_sequenceNum,from, fromLocType, to, toLocType),
busTripRun_sequenceNum(busTripRun_sequenceNum), busline(busline), vehicle_id(vehicle_id), bus_RouteInfo(busRoute_id)
{
	   lastVisitedStop_SequenceNumber = -1;
}

void sim_mob::BusTrip::addBusStopScheduledTimes(const BusStop_ScheduledTimes& aBusStopScheduledTime)
{
	busStopScheduledTimes_vec.push_back(aBusStopScheduledTime);
}

void sim_mob::BusTrip::addBusStopRealTimes(Shared<BusStop_RealTimes>* aBusStopRealTime)
{
	busStopRealTimes_vec.push_back(aBusStopRealTime);
}

void sim_mob::BusTrip::setBusStopRealTimes(int busstopSequence_j, Shared<BusStop_RealTimes>* busStopRealTimes)
{
	//std::cout << "busStopRealTimes " << busStopRealTimes->real_ArrivalTime.getRepr_() << " " << busStopRealTimes->real_DepartureTime.getRepr_() << std::endl;
	if(!busStopRealTimes_vec.empty()) {
		busStopRealTimes_vec[busstopSequence_j] = busStopRealTimes;
	}
}

bool sim_mob::BusTrip::setBusRouteInfo(std::vector<const RoadSegment*>& roadSegment_vec, std::vector<const BusStop*>& busStop_vec)
{
	if(roadSegment_vec.empty()) {
		std::cout << "Error: no roadSegments!!!" << std::endl;
		return false;
	}
	if(busStop_vec.empty()) {
		std::cout << "Error: no busStops!!!" << std::endl;
		// can be, not return
		//return false;
	}
	for(int i = 0; i < roadSegment_vec.size(); i++) {
		bus_RouteInfo.addRoadSegment(roadSegment_vec[i]);
	}
	// later add argument std::vector<const BusStop*>& busStop_vec
	for(int j = 0; j < busStop_vec.size(); j++) {
		bus_RouteInfo.addBusStop(busStop_vec[j]);
	}
	// addBusStopRealTimes and addBusStopScheduledTimes, first time fake Times
	ConfigParams& config = ConfigParams::GetInstance();
	for(int k = 0; k < busStop_vec.size(); k++) {
		Shared<BusStop_RealTimes>* pBusStopRealTimes = new Shared<BusStop_RealTimes>(config.mutexStategy,BusStop_RealTimes());
		addBusStopRealTimes(pBusStopRealTimes);
	}

	std::map<int,std::vector<int> > schedTimes =  ConfigParams::GetInstance().scheduledTImes;
	for(std::map<int,std::vector<int> >::iterator temp=schedTimes.begin();temp != schedTimes.end();temp++)
	{
		BusStop_ScheduledTimes busStop_RealTimes(startTime + DailyTime(temp->second.at(0)),startTime + DailyTime(temp->second.at(1)));
		busStop_RealTimes.Scheduled_busStop = NULL;
		busStopScheduledTimes_vec.push_back(busStop_RealTimes);
		std::cout<<startTime.getValue()<<" "<<temp->second.at(0)<<" "<<busStop_RealTimes.scheduled_ArrivalTime.getValue()<<" "<<busStop_RealTimes.scheduled_ArrivalTime.getValue() - ConfigParams::GetInstance().simStartTime.getValue()<<std::endl;
	}
	std::cout << "busStopRealTimes_vec.size(): " << busStopRealTimes_vec.size() << std::endl;
	return true;
}

sim_mob::Frequency_Busline::Frequency_Busline(DailyTime start_Time, DailyTime end_Time, int headway)
: start_Time(start_Time), end_Time(end_Time), headway(headway)
{

}

sim_mob::Busline::Busline(std::string busline_id, std::string controlType)
: busline_id(busline_id), controlType(getControlTypeFromString(controlType))
{
	control_TimePointNum = 1;// the number 2 in( 0->1->2->3 )
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

void sim_mob::Busline::addFrequencyBusline(const Frequency_Busline& aFrequencyBusline)
{
	frequency_busline.push_back(aFrequencyBusline);
}

void sim_mob::Busline::resetBusTrip_StopRealTimes(int trip_k, int busstopSequence_j, Shared<BusStop_RealTimes>* busStopRealTimes)
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

void sim_mob::PT_Schedule::registerControlType(const std::string busline_id, const CONTROL_TYPE aControlType)
{
	if (buslineID_controlType.count(busline_id)>0) {
		throw std::runtime_error("Duplicate buslineid.");
	}

	buslineID_controlType[busline_id] = aControlType;
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

CONTROL_TYPE sim_mob::PT_Schedule::findBuslineControlType(const std::string& busline_id)
{
	std::map<std::string, CONTROL_TYPE>::const_iterator it;
	it = buslineID_controlType.find(busline_id);
	if (it!=buslineID_controlType.end()) {
		return it->second;
	}
	return NO_CONTROL;
}
