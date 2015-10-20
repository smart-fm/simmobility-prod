//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusTrip.hpp"
#include <boost/lexical_cast.hpp>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/BusStop.hpp"
#include "logging/Log.hpp"


using namespace sim_mob;

sim_mob::BusStopScheduledTimes::BusStopScheduledTimes(DailyTime arrivalTime, DailyTime departureTime)
: arrivalTime(arrivalTime), departureTime(departureTime)
{
	busStop = nullptr;
}

sim_mob::BusStopRealTimes::BusStopRealTimes(DailyTime arrivalTime, DailyTime departureTime)
: arrivalTime(arrivalTime), departureTime(departureTime)
{
	busStop = nullptr;
}

sim_mob::BusStopRealTimes::BusStopRealTimes(const BusStopRealTimes& copyFrom)
: arrivalTime(copyFrom.arrivalTime), departureTime(copyFrom.departureTime)
, busStop(copyFrom.busStop)
{

}

void sim_mob::BusStopRealTimes::setRealBusStop(const BusStop* stop)
{
	busStop = const_cast<BusStop*>(stop);
}

sim_mob::BusRouteInfo::BusRouteInfo(std::string busRouteId)
: busRouteId(busRouteId)
{
	roadSegmentList.clear();
	busStopList.clear();
}

sim_mob::BusRouteInfo::BusRouteInfo(const BusRouteInfo& copyFrom)
: busRouteId(copyFrom.busRouteId), roadSegmentList(copyFrom.roadSegmentList)
, busStopList(copyFrom.busStopList)
{

}

void sim_mob::BusRouteInfo::addBusStop(const BusStop* busStop)
{
	busStopList.push_back(busStop);
}

void sim_mob::BusRouteInfo::addRoadSegment(const RoadSegment* segment)
{
	roadSegmentList.push_back(segment);
}


sim_mob::BusTrip::BusTrip(std::string entId, std::string type,
		unsigned int seqNumber, int requestTime, DailyTime start, DailyTime end,
		int totalSequenceNum, BusLine* busLine, int vehicleId,
		std::string busRouteId, Node* from, std::string fromLocType, Node* to,
		std::string toLocType) :
		Trip(entId, type, seqNumber, requestTime, start, end,
				boost::lexical_cast<std::string>(totalSequenceNum), from,
				fromLocType, to, toLocType), totalSequenceNum(totalSequenceNum), busLine(
				busLine), vehicleId(vehicleId), busRouteInfo(busRouteId) {
	lastVisitedStopSequenceNumber = -1;
}

void sim_mob::BusTrip::addBusStopScheduledTimes(const BusStopScheduledTimes& stopScheduledTime)
{
	busStopScheduledTimes.push_back(stopScheduledTime);
}

void sim_mob::BusTrip::addBusStopRealTimes(Shared<BusStopRealTimes>* realTime)
{
	busStopRealTimes.push_back(realTime);
}

void sim_mob::BusTrip::setBusStopRealTimes(int busstopSequence, Shared<BusStopRealTimes>* realTimes)
{
	if(!busStopRealTimes.empty()) {
		busStopRealTimes[busstopSequence] = realTimes;
	}
}

const int sim_mob::BusTrip::getBusTripStopIndex(const BusStop* stop) const {
	int index = 0;
	const std::vector<const BusStop*>& busStopVec = busRouteInfo.getBusStops();
	for (std::vector<const BusStop*>::const_iterator it = busStopVec.begin();
			it != busStopVec.end(); it++) {
		index++;
		if (stop == (*it)) {
			break;
		}
	}
	return index;
}

bool sim_mob::BusTrip::setBusRouteInfo(const std::vector<const RoadSegment*>& roadSegment_vec, const std::vector<const BusStop*>& busStop_vec)
{
	if(roadSegment_vec.empty()) {
		Warn() << "Error: no roadSegments!!!" << std::endl;
		return false;
	}
	if(busStop_vec.empty()) {
		Warn() << "Error: no busStops!!!" << std::endl;
		return false;
	}
	for(std::vector<const RoadSegment*>::const_iterator it=roadSegment_vec.begin(); it!=roadSegment_vec.end(); it++) {
		busRouteInfo.addRoadSegment(*it);
	}

	for(std::vector<const BusStop*>::const_iterator it=busStop_vec.begin(); it!=busStop_vec.end(); it++) {
		busRouteInfo.addBusStop(*it);
	}

	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	for(int k = 0; k < busStop_vec.size(); k++) {
		Shared<BusStopRealTimes>* pBusStopRealTimes = new Shared<BusStopRealTimes>(config.mutexStategy(),BusStopRealTimes());
		addBusStopRealTimes(pBusStopRealTimes);
	}

    if(config.busController.busLineControlType == "schedule_based"
    		|| config.busController.busLineControlType == "evenheadway_based"
    		|| config.busController.busLineControlType == "hybrid_based")
    {
		std::map<int, BusStopScheduledTime> scheduledTimes =  config.busScheduledTimes;
		for(std::map<int, BusStopScheduledTime>::iterator temp=scheduledTimes.begin();temp != scheduledTimes.end();temp++)
		{
			BusStopScheduledTimes busStop_ScheduledTimes(startTime + DailyTime(temp->second.offsetAT),startTime + DailyTime(temp->second.offsetDT));
			addBusStopScheduledTimes(busStop_ScheduledTimes);
		}
	}
	return true;
}

sim_mob::FrequencyBusLine::FrequencyBusLine(DailyTime start_Time, DailyTime end_Time, int headway)
: startTime(start_Time), endTime(end_Time), headwaySec(headway)
{

}

sim_mob::BusLine::BusLine(std::string buslineId, std::string cType)
: busLineId(buslineId), controlType(getControlTypeFromString(cType))
{
	controlTimePointNum0 = 15;// the number 2 in( 0->1->2->3 )
	controlTimePointNum1 = 21;
	controlTimePointNum2 = 39;
	controlTimePointNum3 = 51;
}

sim_mob::BusLine::~BusLine()
{
	for(size_t i = 0; i < busTrips.size(); i++) {
		busTrips[i].safeDeleteBusStopRealTimes();
	}
}

ControlTypes sim_mob::BusLine::getControlTypeFromString(std::string controlType)
{
	controlType.erase(remove_if(controlType.begin(), controlType.end(), isspace),
			controlType.end());
	if (controlType == "no_control") {
		return NO_CONTROL;
	} else if (controlType == "schedule_based") {
		return SCHEDULE_BASED;
	} else if (controlType == "headway_based") {
		return HEADWAY_BASED;
	} else if (controlType == "evenheadway_based") {
		return EVENHEADWAY_BASED;
	} else if (controlType == "hybrid_based") {
		return HYBRID_BASED;
	} else {
		throw std::runtime_error("Unexpected control type.");
	}
}

void sim_mob::BusLine::addBusTrip(BusTrip& trip)
{
	busTrips.push_back(trip);
}

void sim_mob::BusLine::addFrequencyBusLine(const FrequencyBusLine& frequency)
{
	frequencyBusLine.push_back(frequency);
}

void sim_mob::BusLine::resetBusTripStopRealTimes(int trip, int sequence, Shared<BusStopRealTimes>* realTimes)
{
	if(!busTrips.empty()) {
		busTrips[trip].setBusStopRealTimes(sequence, realTimes);
	}
}

sim_mob::PT_Schedule::PT_Schedule()
{

}

sim_mob::PT_Schedule::~PT_Schedule()
{
	std::map<std::string, BusLine*>::iterator it;
	for (it = buslineIdBuslineMap.begin(); it != buslineIdBuslineMap.end(); ++it) {
		safe_delete_item(it->second);
	}
	buslineIdBuslineMap.clear();
}

void sim_mob::PT_Schedule::registerBusLine(const std::string lineId, BusLine* line)
{
	if (buslineIdBuslineMap.count(lineId)>0) {
		throw std::runtime_error("Duplicate buslineid.");
	}

	buslineIdBuslineMap[lineId] = line;
}

void sim_mob::PT_Schedule::registerControlType(const std::string buslineId, const ControlTypes controlType)
{
	if (buslineIdControlTypeMap.count(buslineId)>0) {
		throw std::runtime_error("Duplicate buslineId.");
	}

	buslineIdControlTypeMap[buslineId] = controlType;
}

BusLine* sim_mob::PT_Schedule::findBusLine(const std::string& buslineId)
{
	std::map<std::string, BusLine*>::const_iterator it;
	it = buslineIdBuslineMap.find(buslineId);
	if (it!=buslineIdBuslineMap.end()) {
		return it->second;
	}
	return nullptr;
}

ControlTypes sim_mob::PT_Schedule::findBusLineControlType(const std::string& buslineId)
{
	std::map<std::string, ControlTypes>::const_iterator it;
	it = buslineIdControlTypeMap.find(buslineId);
	if (it!=buslineIdControlTypeMap.end()) {
		return it->second;
	}
	return NO_CONTROL;
}
