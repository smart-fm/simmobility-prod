//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusController.hpp"

#include <stdexcept>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/misc/BusTrip.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"
#include "util/LangHelpers.hpp"
#include "boost/algorithm/string.hpp"

using std::vector;
using std::string;
using std::map;

using namespace sim_mob;

typedef Entity::UpdateStatus UpdateStatus;


BusController* BusController::instance = nullptr;

namespace
{
// planned headway(ms) of the buses
const double PLANNED_HEADWAY_MS = 480000;
// secs conversion unit from milliseconds
const double SECS_CONVERT_UNIT = 0.001;
// millisecs conversion unit from seconds
const double MILLISECS_CONVERT_UNIT = 1000.0;
}

void sim_mob::BusController::RegisterBusController(unsigned int startTime, const MutexStrategy& mtxStrat)
{
	if(!instance){
		instance = new sim_mob::BusController(-1, mtxStrat);
		instance->setStartTime(startTime);
	}
}

bool sim_mob::BusController::HasBusControllers()
{
	return (instance!=nullptr);
}

void sim_mob::BusController::InitializeAllControllers(std::set<Entity*>& agentList, const vector<PT_BusDispatchFreq>& dispatchFreq)
{
	if (!instance) {
		throw std::runtime_error("create bus controller before you want to initialize");
	}

	instance->setPTScheduleFromConfig(dispatchFreq);
	instance->assignBusTripChainWithPerson(agentList);
}


void sim_mob::BusController::DispatchAllControllers(std::set<Entity*>& agentList)
{
	//Push every item on the list into the agents array as an active agent
	if(instance) {
		agentList.insert(instance);
	}
}


BusController* sim_mob::BusController::GetInstance()
{
	if(!instance){
		instance = new sim_mob::BusController();
	}
	return instance;
}

std::vector<BufferedBase *> sim_mob::BusController::buildSubscriptionList()
{
	return Agent::buildSubscriptionList();
}

void sim_mob::BusController::CollectAndProcessAllRequests()
{
	if (instance)
	{
		instance->handleChildrenRequest();
	}
}

void sim_mob::BusController::assignBusTripChainWithPerson(std::set<sim_mob::Entity*>& activeAgents)
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const map<string, BusLine*>& buslines = ptSchedule.getBusLines();
	if (0 == buslines.size())
	{
		throw std::runtime_error("Error:  No busline in the ptSchedule, please check the setPTSchedule.");
	}

	for (map<string, BusLine*>::const_iterator buslinesIt = buslines.begin(); buslinesIt != buslines.end(); buslinesIt++)
	{
		BusLine* busline = buslinesIt->second;
		const vector<BusTrip>& busTrips = busline->queryBusTrips();

		for (vector<BusTrip>::const_iterator tripIt = busTrips.begin(); tripIt != busTrips.end(); tripIt++)
		{
			if (tripIt->startTime.isAfterEqual(ConfigManager::GetInstance().FullConfig().simStartTime()))
			{
				Person* person = new Person("BusController", config.mutexStategy(), -1, tripIt->getPersonID());
				person->setPersonCharacteristics();
				vector<TripChainItem*> tripChain;
				tripChain.push_back(const_cast<BusTrip*>(&(*tripIt)));
				person->setTripChain(tripChain);
				person->initTripChain();
				addOrStashBuses(person, activeAgents);
			}
		}
	}

	for (std::set<Entity*>::iterator it = activeAgents.begin(); it != activeAgents.end(); it++)
	{
		(*it)->parentEntity = this;
		allChildren.push_back(*it);
	}
}

struct RouteInfo{
	std::string line;
	unsigned int id;
	unsigned int index;
	std::string start;
	std::string end;
	unsigned int startPosX;
	unsigned int startPosY;
	unsigned int endPosX;
	unsigned int endPosY;
};

struct StopInfo
{
	std::string line;
	std::string id;
	unsigned int posX;
	unsigned int posY;
	int index;
};

bool searchBusRoutes(const vector<const BusStop*>& stops,
					 const std::string& busLine, std::deque<RouteInfo>& allRoutes,
					 std::deque<StopInfo>& allStops)
{

	const BusStop* start;
	const BusStop* end;
	const BusStop* nextEnd;
	bool isFound = true;
	if (stops.size() > 0)
	{
		start = nullptr;
		end = nullptr;
		nextEnd = nullptr;
		std::deque<RouteInfo> routeIDs;
		std::deque<StopInfo> stopIDs;
		for (int k = 0; k < stops.size(); k++)
		{
			isFound = false;
			const BusStop* busStop = stops[k];
			if (k == 0)
			{
				start = busStop;
				StopInfo stopInfo;
				stopInfo.id = start->getBusstopno_();
				stopInfo.line = busLine;
				stopInfo.posX = start->xPos;
				stopInfo.posY = start->yPos;
				stopIDs.push_back(stopInfo);
			}
			else
			{
				end = busStop;
				const StreetDirectory& stdir = StreetDirectory::instance();
				StreetDirectory::VertexDesc startDes = stdir.DrivingVertex(*start);
				StreetDirectory::VertexDesc endDes = stdir.DrivingVertex(*end);
				vector<WayPoint> path;
				if (start->getParentSegment() == end->getParentSegment())
				{
					path.push_back(WayPoint(start->getParentSegment()));
				}
				else
				{
					path = stdir.SearchShortestDrivingPath(startDes, endDes);
				}

				for (std::vector<WayPoint>::const_iterator it = path.begin();
						it != path.end(); it++)
				{
					if (it->type_ == WayPoint::ROAD_SEGMENT)
					{
						unsigned int id =
								(*it).roadSegment_->getSegmentAimsunId();
						if (routeIDs.size() == 0 || routeIDs.back().id != id)
						{
							RouteInfo route;
							route.id = id;
							route.start = start->getBusstopno_();
							route.end = end->getBusstopno_();
							route.startPosX = start->xPos;
							route.startPosY = start->yPos;
							route.endPosX = end->xPos;
							route.endPosY = end->yPos;
							routeIDs.push_back(route);
						}
						isFound = true;
					}
				}

				if (!isFound)
				{
					std::cout << "can not find bus route in bus line:" << busLine
							<< " start stop:" << start->getBusstopno_()
							<< "  end stop:" << end->getBusstopno_()
							<< std::endl;
					routeIDs.clear();
					stopIDs.clear();
					break;
				}
				else
				{
					StopInfo stopInfo;
					stopInfo.id = end->getBusstopno_();
					stopInfo.line = busLine;
					stopInfo.posX = end->xPos;
					stopInfo.posY = end->yPos;
					stopIDs.push_back(stopInfo);
				}

				start = end;
			}
		}

		if (routeIDs.size() > 0 && stopIDs.size() > 0)
		{
			unsigned int index = 0;
			for (std::deque<RouteInfo>::const_iterator it = routeIDs.begin();
					it != routeIDs.end(); it++)
			{
				RouteInfo routeInfo;
				routeInfo.line = busLine;
				routeInfo.id = it->id;
				routeInfo.start = it->start;
				routeInfo.end = it->end;
				routeInfo.startPosX = it->startPosX;
				routeInfo.startPosY = it->startPosY;
				routeInfo.endPosX = it->endPosX;
				routeInfo.endPosY = it->endPosY;
				routeInfo.index = index;
				allRoutes.push_back(routeInfo);
				index++;
			}

			index = 0;
			for (std::deque<StopInfo>::const_iterator it = stopIDs.begin();
					it != stopIDs.end(); it++)
			{
				StopInfo stopInfo;
				stopInfo.line = it->line;
				stopInfo.id = it->id;
				stopInfo.index = index;
				stopInfo.posX = it->posX;
				stopInfo.posY = it->posY;
				allStops.push_back(stopInfo);
				index++;
			}
		}
	}

	return isFound;
}

void sim_mob::BusController::setPTScheduleFromConfig(const vector<PT_BusDispatchFreq>& dispatchFreq)
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	vector<const BusStop*> stops;
	sim_mob::BusLine* busline = nullptr;
	int step = 0;
	allChildren.clear();
	bool busLineRegistered=false;
	DailyTime lastBusDispatchTime;
	std::deque<RouteInfo> allRoutes;
	std::deque<StopInfo> allStops;

	for (vector<sim_mob::PT_BusDispatchFreq>::const_iterator curr=dispatchFreq.begin(); curr!=dispatchFreq.end(); curr++) {
		vector<sim_mob::PT_BusDispatchFreq>::const_iterator next = curr+1;

		//If we're on a new BusLine, register it with the scheduler.
		if(!busline || (curr->routeId != busline->getBusLineID())) {
			busline = new sim_mob::BusLine(curr->routeId,config.busline_control_type());
			ptSchedule.registerBusLine(curr->routeId, busline);
			ptSchedule.registerControlType(curr->routeId, busline->getControlType());
			step = 0;
			busLineRegistered = true;
		}

		// define frequency_busline for one busline
		busline->addFrequencyBusLine(FrequencyBusLine(curr->startTime,curr->endTime,curr->headwaySec));

		//Set nextTime to the next frequency bus line's start time or the current line's end time if this is the last line.
		sim_mob::DailyTime nextTime = curr->endTime;

		DailyTime advance(curr->headwaySec*MILLISECS_CONVERT_UNIT);
		for(DailyTime startTime = curr->startTime; startTime.isBeforeEqual(nextTime); startTime += advance) {
			// deal with small gaps between the group dispatching times
			if ((startTime - lastBusDispatchTime).isBeforeEqual(advance))
			{
				startTime = lastBusDispatchTime + advance;
			}

			BusTrip bustrip("", "BusTrip", 0, -1, startTime, DailyTime("00:00:00"), step++, busline, -1, curr->routeId, nullptr, "node", nullptr, "node");

			//Try to find our data.
			map<string, vector<const RoadSegment*> >::const_iterator segmentsIt = config.getRoadSegments_Map().find(curr->routeId);
			map<string, vector<const BusStop*> >::const_iterator stopsIt = config.getBusStops_Map().find(curr->routeId);

			vector<const RoadSegment*> segments = vector<const RoadSegment*>();
			const std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& routeID_roadSegments = config.getRoadSegments_Map();
			map<string, vector<const RoadSegment*> >::const_iterator itSeg = routeID_roadSegments.find(curr->routeId);
			if (itSeg != routeID_roadSegments.end()) {
				segments = itSeg->second;
			}

			stops = vector<const BusStop*>();
			const std::map<std::string, std::vector<const sim_mob::BusStop*> >& routeID_busStops = config.getBusStops_Map();
			map<string, vector<const BusStop*> >::const_iterator itStop = routeID_busStops.find(curr->routeId);
			if (itStop != routeID_busStops.end()) {
				stops = itStop->second;
			}

			if (busLineRegistered
					&& sim_mob::ConfigManager::GetInstance().FullConfig().isGenerateBusRoutes()) {
				searchBusRoutes(stops, curr->routeId, allRoutes,allStops);
			}

			if (busLineRegistered)
			{
				for (int k = 0; k < stops.size(); k++) {
					//to store the bus line info at each bus stop
					BusStop* busStop = const_cast<BusStop*>(stops[k]);
					busStop->BusLines.push_back(busline);
				}
				busLineRegistered = false;
			}
			if (bustrip.setBusRouteInfo(segments, stops)) {
				busline->addBusTrip(bustrip);
			}
			lastBusDispatchTime = startTime;
		}
	}

	if (sim_mob::ConfigManager::GetInstance().FullConfig().isGenerateBusRoutes())
	{

		std::ofstream outputRoutes("routes.csv");
		for (std::deque<RouteInfo>::const_iterator it =
				allRoutes.begin(); it != allRoutes.end(); it++)
		{
			if (outputRoutes.is_open())
			{
				outputRoutes << it->line << ","
						<< it->index << ","
						<< it->id
						<< std::endl;
			}
		}
		outputRoutes.close();

		std::ofstream outputStop("stops.csv");
		for (std::deque<StopInfo>::const_iterator it =
				allStops.begin(); it != allStops.end(); it++)
		{
			if (outputStop.is_open())
			{
				outputStop << it->line << ","
						<< it->id << ","
						<< it->index
						<< std::endl;
			}
		}
		outputStop.close();
	}
}

void sim_mob::BusController::storeRealTimesAtEachBusStop(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, const BusStop* lastVisitedBusStop, BusStopRealTimes& realTime)
{
	BusLine* busline = ptSchedule.findBusLine(busLine);
	if(!busline) {
		return;
	}

	double departureTime = arrivalTime + (departTime * MILLISECS_CONVERT_UNIT);
	BusStopRealTimes busStopRealTimes(ConfigManager::GetInstance().FullConfig().simStartTime() + DailyTime(arrivalTime), ConfigManager::GetInstance().FullConfig().simStartTime() + DailyTime(departureTime));
	busStopRealTimes.setRealBusStop(lastVisitedBusStop);
	realTime = (busStopRealTimes);

	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	Shared<BusStopRealTimes>* sharedStopRealTimes = new Shared<BusStopRealTimes>(config.mutexStategy(), busStopRealTimes);
	// set this value for next step
	busline->resetBusTripStopRealTimes(trip, sequence, sharedStopRealTimes);
}

double sim_mob::BusController::decisionCalculation(const string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisitedBusStop)
{
	ControlTypes controlType = ptSchedule.findBusLineControlType(busLine);
	BusLine* busline = ptSchedule.findBusLine(busLine);
	if(!busline) {
		return -1;
	}

	const vector<BusTrip>& busTrips = busline->queryBusTrips();
	double departureTime = 0;
	double waitTimeBusStop = 0;

	switch (controlType)
	{
	case SCHEDULE_BASED:
		departureTime = scheduledDecision(busLine, trip, sequence, arrivalTime, departTime, realTime, lastVisitedBusStop);
		waitTimeBusStop = (departureTime - arrivalTime) * SECS_CONVERT_UNIT;
		break;
	case HEADWAY_BASED:
		departureTime = headwayDecision(busLine, trip, sequence, arrivalTime, departTime, realTime, lastVisitedBusStop);
		waitTimeBusStop = (departureTime - arrivalTime) * SECS_CONVERT_UNIT;
		break;
	case EVENHEADWAY_BASED:
		departureTime = evenheadwayDecision(busLine, trip, sequence, arrivalTime, departTime, realTime, lastVisitedBusStop);
		waitTimeBusStop = (departureTime - arrivalTime) * SECS_CONVERT_UNIT;
		break;
	case HYBRID_BASED:
		departureTime = hybridDecision(busLine, trip, sequence, arrivalTime, departTime, realTime, lastVisitedBusStop);
		waitTimeBusStop = (departureTime - arrivalTime) * SECS_CONVERT_UNIT;
		break;
	default:
		storeRealTimesAtEachBusStop(busLine, trip, sequence, arrivalTime, departTime, lastVisitedBusStop, realTime);
		waitTimeBusStop = departTime;
		break;
	}
	return waitTimeBusStop;
}

double sim_mob::BusController::scheduledDecision(const string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisitedBusStop)
{
	BusLine* busline = ptSchedule.findBusLine(busLine);
	if(!busline) {
		return -1;
	}

	double assumedTime = 0;
	double estimatedTime = 0;

	const vector<BusTrip>& busTrips = busline->queryBusTrips();
	const vector<BusStopScheduledTimes>& busStopScheduledTime = busTrips[trip].getBusStopScheduledTimes();
	assumedTime = busStopScheduledTime[sequence].departureTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());
	estimatedTime = std::max(assumedTime, arrivalTime + (departTime * MILLISECS_CONVERT_UNIT));

	storeRealTimesAtEachBusStop(busLine, trip, sequence, arrivalTime, departTime, lastVisitedBusStop, realTime);

	return estimatedTime;
}

double sim_mob::BusController::headwayDecision(const string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisitedBusStop)
{
	BusLine* busline = ptSchedule.findBusLine(busLine);
	if (!busline) {
		return -1;
	}

	double estimatedTime = 0;
	double arrivalTimeMinusOne = 0;
	double alpha = 0.8;

	const vector<BusTrip>& busTrips = busline->queryBusTrips();
	const vector<Shared<BusStopRealTimes>*>& busStopRealTimeTripkMinusOne = busTrips[trip - 1].getBusStopRealTimes();

	// data has already updated
	if (busStopRealTimeTripkMinusOne[sequence]->get().busStop) {
		// there are some cases that buses are bunched together so that k-1 has no values updated yet
		arrivalTimeMinusOne = busStopRealTimeTripkMinusOne[sequence]->get().arrivalTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());
		estimatedTime = std::max(arrivalTimeMinusOne + alpha * PLANNED_HEADWAY_MS, arrivalTime + (departTime * MILLISECS_CONVERT_UNIT));
	} else {
		// immediately leave
		estimatedTime = arrivalTime + (departTime * MILLISECS_CONVERT_UNIT);
	}

	storeRealTimesAtEachBusStop(busLine, trip, sequence, arrivalTime, departTime, lastVisitedBusStop, realTime);

	return estimatedTime;
}

double sim_mob::BusController::evenheadwayDecision(const string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisitedBusStop)
{
	BusLine* busline = ptSchedule.findBusLine(busLine);
	if (!busline) {
		return -1;
	}
	const vector<BusTrip>& busTrips = busline->queryBusTrips();

	double estimatedTime = 0;
	double arrivalTimeMinusOne = 0;
	double oneTimePlusOne = 0;

	//check if last trip
	bool lastTrip = ((busTrips.size() - 1) == trip);
	// check whether last visited Stop num is valid or not
	int lastVisitedStopNum = 0;
	if (!lastTrip) {
		lastVisitedStopNum = busTrips[trip + 1].getLastStopSequenceNumber();
	}

	if (0 == trip) {
		// the first trip just use Dwell Time, no holding strategy
		estimatedTime = arrivalTime + (departTime * MILLISECS_CONVERT_UNIT);

	} else if (lastTrip || lastVisitedStopNum == -1) {
		// If last trip or if next trip k+1 is not dispatched yet then use single headway
		return headwayDecision(busLine, trip, sequence, arrivalTime, departTime, realTime, lastVisitedBusStop);
	} else {
		//last stop visited by bus trip k+1
		lastVisitedStopNum = busTrips[trip + 1].getLastStopSequenceNumber();
		const vector<Shared<BusStopRealTimes>*>& busStopRealTimeTripkMinusOne = busTrips[trip - 1].getBusStopRealTimes();
		arrivalTimeMinusOne = busStopRealTimeTripkMinusOne[sequence]->get().arrivalTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());

		const vector<Shared<BusStopRealTimes>*>& busStopRealTime_tripKplus1 = busTrips[trip + 1].getBusStopRealTimes();
		oneTimePlusOne = busStopRealTime_tripKplus1[lastVisitedStopNum]->get().arrivalTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());

		const vector<BusStopScheduledTimes>& busStopScheduledTime_tripKplus1 = busTrips[trip + 1].getBusStopScheduledTimes();
		double val = busStopScheduledTime_tripKplus1[sequence].arrivalTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime())
						- busStopScheduledTime_tripKplus1[lastVisitedStopNum].departureTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());

		estimatedTime = std::max(arrivalTimeMinusOne + (oneTimePlusOne + val - arrivalTimeMinusOne) / 2.0,
				arrivalTime + (departTime * MILLISECS_CONVERT_UNIT));
	}

	storeRealTimesAtEachBusStop(busLine, trip, sequence, arrivalTime, departTime, lastVisitedBusStop, realTime);

	return estimatedTime;
}

double sim_mob::BusController::hybridDecision(const string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisitedBusStop)
{
	BusLine* busline = ptSchedule.findBusLine(busLine);
	if (!busline) {
		return -1;
	}

	const vector<BusTrip>& busTrips = busline->queryBusTrips();
	double estimatedTime = 0;
	double arrivalTimeMinusOne = 0;
	double oneTimePlusOne = 0;
	//check if last trip
	bool lastTrip = ((busTrips.size() - 1) == trip);
	// check whether last visited Stop num is valid or not
	int lastVisitedStopNum = 0;
	if (!lastTrip) {
		lastVisitedStopNum = busTrips[trip + 1].getLastStopSequenceNumber();
	}

	if (0 == trip) {
		// the first trip just use Dwell Time, no holding strategy
		estimatedTime = arrivalTime + (departTime * MILLISECS_CONVERT_UNIT);

	} else if (lastTrip || lastVisitedStopNum == -1) {
		// If last trip or if next trip k+1 is not dispatched yet then use single headway
		return headwayDecision(busLine, trip, sequence, arrivalTime, departTime, realTime, lastVisitedBusStop);
	} else {
		lastVisitedStopNum = busTrips[trip + 1].getLastStopSequenceNumber();
		const vector<Shared<BusStopRealTimes>*>& busStopRealTimeTripkMinusOne = busTrips[trip - 1].getBusStopRealTimes();
		arrivalTimeMinusOne = busStopRealTimeTripkMinusOne[sequence]->get().arrivalTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());

		const vector<Shared<BusStopRealTimes>*>& busStopRealTime_tripKplus1 = busTrips[trip + 1].getBusStopRealTimes();
		oneTimePlusOne = busStopRealTime_tripKplus1[lastVisitedStopNum]->get().arrivalTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());

		const vector<BusStopScheduledTimes>& busStopScheduledTime_tripKplus1 =	busTrips[trip + 1].getBusStopScheduledTimes();
		double val = busStopScheduledTime_tripKplus1[sequence].arrivalTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime())
						- busStopScheduledTime_tripKplus1[lastVisitedStopNum].departureTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());
		estimatedTime = std::max(
				std::min(arrivalTimeMinusOne + (oneTimePlusOne + val - arrivalTimeMinusOne)/ 2.0, (arrivalTimeMinusOne + PLANNED_HEADWAY_MS)),
				(double) (arrivalTime) + (departTime * MILLISECS_CONVERT_UNIT));
	}

	storeRealTimesAtEachBusStop(busLine, trip, sequence, arrivalTime, departTime, lastVisitedBusStop, realTime);

	return estimatedTime;
}

void sim_mob::BusController::addOrStashBuses(Agent* p, std::set<Entity*>& activeAgents)
{
	if (p->getStartTime()==0) {
		//Only agents with a start time of zero should start immediately in the all_agents list.
		p->load(p->getConfigProperties());
		p->clearConfigProperties();
		activeAgents.insert(p);
	} else {
		//Start later.
		pendingChildren.push(p);
	}
}

void sim_mob::BusController::handleEachChildRequest(sim_mob::DriverRequestParams rParams)
{
	//No reaction if request params is empty.
	if (rParams.asVector().empty()) {
		return;
	}

	Shared<int>* existedRequestMode = rParams.existedRequest_Mode;
	if (existedRequestMode
			&& (existedRequestMode->get() == Role::REQUEST_DECISION_TIME || existedRequestMode->get() == Role::REQUEST_STORE_ARRIVING_TIME)) {
		Shared<string>* lastVisitedBusline = rParams.lastVisited_Busline;
		Shared<int>* lastVisitedBusTripSequenceNo = rParams.lastVisited_BusTrip_SequenceNo;
		Shared<int>* busstopSequenceNo = rParams.busstop_sequence_no;
		Shared<double>* realArrivalTime = rParams.real_ArrivalTime;
		Shared<double>* dwellTime = rParams.DwellTime_ijk;
		Shared<const BusStop*>* lastVisitedBusStop = rParams.lastVisited_BusStop;
		Shared<BusStopRealTimes>* lastBusStopRealTimes = rParams.last_busStopRealTimes;
		Shared<double>* waitingTime = rParams.waiting_Time;

		if (existedRequestMode && lastVisitedBusline
				&& lastVisitedBusTripSequenceNo && busstopSequenceNo
				&& realArrivalTime && dwellTime && lastVisitedBusStop
				&& lastBusStopRealTimes && waitingTime) {
			BusStopRealTimes realTime;
			if (existedRequestMode->get() == Role::REQUEST_DECISION_TIME) {
				double waitingtime = decisionCalculation(
						lastVisitedBusline->get(),
						lastVisitedBusTripSequenceNo->get(),
						busstopSequenceNo->get(), realArrivalTime->get(),
						dwellTime->get(), realTime,
						lastVisitedBusStop->get());
				waitingTime->set(waitingtime);
			} else if (existedRequestMode->get() == Role::REQUEST_STORE_ARRIVING_TIME) {
				storeRealTimesAtEachBusStop(lastVisitedBusline->get(),
						lastVisitedBusTripSequenceNo->get(),
						busstopSequenceNo->get(), realArrivalTime->get(),
						dwellTime->get(), lastVisitedBusStop->get(),
						realTime);
			}
			lastBusStopRealTimes->set(realTime);
		}
	}
}


void sim_mob::BusController::handleChildrenRequest() {
	for (vector<Entity*>::iterator it = allChildren.begin();
			it != allChildren.end(); it++) {
		Person* person = dynamic_cast<sim_mob::Person*>(*it);
		if (person) {
			Role* role = person->getRole();
			if (role) {
				handleEachChildRequest(role->getDriverRequestParams());
			}
		}
	}
}

void sim_mob::BusController::unregisteredChild(Entity* child) {
	if (child) {
		std::vector<Entity*>::iterator it = std::find(allChildren.begin(),
				allChildren.end(), child);
		if (it != allChildren.end()) {
			allChildren.erase(it);
		}
	}
}

Entity::UpdateStatus sim_mob::BusController::frame_tick(timeslice now)
{
	nextTimeTickToStage++;
	unsigned int nextTickMS = (nextTimeTickToStage+3)*ConfigManager::GetInstance().FullConfig().baseGranMS();

	//Stage any pending entities that will start during this time tick.
	while (!pendingChildren.empty()
			&& pendingChildren.top()->getStartTime() <= nextTickMS) {
		//Ask the current worker's parent WorkGroup to schedule this Entity.
		Agent* child = pendingChildren.top();
		pendingChildren.pop();
		child->parentEntity = this;
		currWorkerProvider->scheduleForBred(child);
		allChildren.push_back(child);
	}

	handleChildrenRequest();

	return Entity::UpdateStatus::Continue;
}

bool sim_mob::BusController::frame_init(timeslice now)
{
	return true;
}

void sim_mob::BusController::frame_output(timeslice now)
{
	LogOut("(\"BusController\""
			<<","<<now.frame()
			<<","<<getId()
			<<","<<std::endl);
}
