//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusController.hpp"

#include <boost/noncopyable.hpp>
#include <map>
#include <stdexcept>
#include <vector>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/misc/BusTrip.hpp"
#include "entities/misc/PublicTransit.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/SOCI_Converters.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/Log.hpp"
#include "geospatial/streetdir/A_StarShortestPathImpl.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"
#include "util/LangHelpers.hpp"
#include "entities/misc/TripChain.hpp"

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
const double SECS_IN_UNIT_MS = 0.001;
// millisecs conversion unit from seconds
const double MS_IN_UNIT_SEC = 1000.0;

/**
 * Database loader for bus controller
 */
class DbLoader : private boost::noncopyable
{
private:
	soci::session sql_;

public:
	explicit DbLoader(string const & connectionString);

	void loadPTBusDispatchFreq(const std::string& storedProc, std::vector<sim_mob::PT_BusDispatchFreq>& ptBusDispatchFreq);
	void loadPTBusRoutes(const std::string& storedProc,
			std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& routeID_roadSegments);
	void loadPTBusStops(const std::string& storedProc,
			std::map<std::string, std::vector<const sim_mob::BusStop*> >& routeID_busStops,
			std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& routeID_roadSegments);
};

DbLoader::DbLoader(string const & connectionString)
: sql_(soci::postgresql, connectionString)
{
}

void DbLoader::loadPTBusDispatchFreq(const std::string& storedProc, std::vector<sim_mob::PT_BusDispatchFreq>& ptBusDispatchFreq)
{
	if (storedProc.empty())
	{
		sim_mob::Warn() << "WARNING: An empty 'PT_BusDispatchFreq' stored-procedure was specified in the config file; " << std::endl;
		return;
	}
	soci::rowset<sim_mob::PT_BusDispatchFreq> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<sim_mob::PT_BusDispatchFreq>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		//sim_mob::PT_bus_dispatch_freq* pt_bus_freqTemp = new sim_mob::PT_bus_dispatch_freq(*iter);
		sim_mob::PT_BusDispatchFreq pt_bus_freqTemp = *iter;
		pt_bus_freqTemp.routeId.erase(remove_if(pt_bus_freqTemp.routeId.begin(), pt_bus_freqTemp.routeId.end(), ::isspace),
				pt_bus_freqTemp.routeId.end());
		pt_bus_freqTemp.frequencyId.erase(remove_if(pt_bus_freqTemp.frequencyId.begin(), pt_bus_freqTemp.frequencyId.end(), ::isspace),
				pt_bus_freqTemp.frequencyId.end());
		ptBusDispatchFreq.push_back(pt_bus_freqTemp);
	}
}

void DbLoader::loadPTBusRoutes(const std::string& storedProc,
		std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& routeID_roadSegments)
{
	if (storedProc.empty())
	{
		sim_mob::Warn() << "WARNING: An empty 'pt_bus_routes' stored-procedure was specified in the config file; " << std::endl;
		return;
	}
	const sim_mob::RoadNetwork* rn = sim_mob::RoadNetwork::getInstance();
	soci::rowset<sim_mob::PT_BusRoutes> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<sim_mob::PT_BusRoutes>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		sim_mob::PT_BusRoutes& pt_bus_routesTemp = *iter;
		const sim_mob::RoadSegment *seg = rn->getById(rn->getMapOfIdVsRoadSegments(), atoi(pt_bus_routesTemp.linkId.c_str()));		
		if(seg) {
			routeID_roadSegments[iter->routeId].push_back(seg);
		}
	}
}

void DbLoader::loadPTBusStops(const std::string& storedProc,
		std::map<std::string, std::vector<const sim_mob::BusStop*> >& routeID_busStops,
		std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& routeID_roadSegments)
{

	sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();
	if (storedProc.empty())
	{
		sim_mob::Warn() << "WARNING: An empty 'pt_bus_stops' stored-procedure was specified in the config file; " << std::endl;
		return;
	}
	soci::rowset<sim_mob::PT_BusStops> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<sim_mob::PT_BusStops>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		sim_mob::PT_BusStops& pt_bus_stopsTemp = *iter;

		sim_mob::BusStop* bs = sim_mob::BusStop::findBusStop(pt_bus_stopsTemp.stopNo);
		if(bs) {
			routeID_busStops[iter->routeId].push_back(bs);
		}
	}

	for(std::map<std::string, std::vector<const sim_mob::BusStop*> >::iterator routeIt=routeID_busStops.begin();
			routeIt!=routeID_busStops.end(); routeIt++)
	{
		std::map<std::string, std::vector<const sim_mob::RoadSegment*> >::iterator routeIDSegIt = routeID_roadSegments.find(routeIt->first);
		if(routeIDSegIt == routeID_roadSegments.end())
		{
			sim_mob::Warn() << routeIt->first << " has no route";
			continue;
		}
		std::vector<const sim_mob::BusStop*>& stopList = routeIt->second;
		std::vector<const sim_mob::RoadSegment*>& segList = routeIDSegIt->second;

		if(stopList.empty()) { throw std::runtime_error("empty stopList!"); }
		std::vector<const sim_mob::BusStop*> stopListCopy = stopList; //copy locally
		stopList.clear(); //empty stopList

		const sim_mob::BusStop* firstStop = stopListCopy.front();
		if(firstStop->getTerminusType() == sim_mob::SINK_TERMINUS)
		{
			const sim_mob::BusStop* firstStopTwin = firstStop->getTwinStop();
			if(!firstStopTwin) { throw std::runtime_error("Sink bus stop found without a twin!"); }
			stopList.push_back(firstStopTwin);
			if(!segList.empty())
			{
				std::vector<const sim_mob::RoadSegment*>::iterator itToDelete = segList.begin();
				while(itToDelete!=segList.end() && (*itToDelete) != firstStopTwin->getParentSegment())
				{
					itToDelete = segList.erase(itToDelete); // the bus must start from the segment of the twinStop
				}
				if(segList.empty())
				{
					throw std::runtime_error("Bus route violates terminus assumption. Entire route was deleted");
				}
			}
		}
		else
		{
			stopList.push_back(firstStop);
		}

		for(size_t stopIt = 1; stopIt < (stopListCopy.size()-1); stopIt++) //iterate through all stops but the first and last
		{
			const sim_mob::BusStop* stop = stopListCopy[stopIt];
			switch(stop->getTerminusType())
			{
				case sim_mob::NOT_A_TERMINUS:
				{
					stopList.push_back(stop);
					break;
				}
				case sim_mob::SOURCE_TERMINUS:
				{
					const sim_mob::BusStop* stopTwin = stop->getTwinStop();
					if(!stopTwin) { throw std::runtime_error("Source bus stop found without a twin!"); }
					stopList.push_back(stopTwin);
					stopList.push_back(stop);
					break;
				}
				case sim_mob::SINK_TERMINUS:
				{
					const sim_mob::BusStop* stopTwin = stop->getTwinStop();
					if(!stopTwin) { throw std::runtime_error("Sink bus stop found without a twin!"); }
					stopList.push_back(stop);
					stopList.push_back(stopTwin);
					break;
				}
			}
		}

		if(stopListCopy.size() > 1)
		{
			const sim_mob::BusStop* lastStop = stopListCopy[stopListCopy.size() - 1];
			if (lastStop->getTerminusType() == sim_mob::SOURCE_TERMINUS)
			{
				const sim_mob::BusStop* lastStopTwin = lastStop->getTwinStop();
				if (!lastStopTwin)
				{
					throw std::runtime_error("Source bus stop found without a twin!");
				}
				stopList.push_back(lastStopTwin);
				if (!segList.empty())
				{
					std::vector<const sim_mob::RoadSegment*>::iterator itToDelete = --segList.end();
					while ((*itToDelete) != lastStopTwin->getParentSegment())
					{
						itToDelete = segList.erase(itToDelete); //the bus must end at the segment of twin stop
						itToDelete--; //itToDelete will be segList.end(); so decrement to get last valid iterator
					}
					if (segList.empty())
					{
						throw std::runtime_error("Bus route violates terminus assumption. Entire route was deleted");
					}
				}
			}
			else
			{
				stopList.push_back(lastStop);
			}
		}
	}
}
} //anon namespace

bool BusController::RegisterBusController(BusController* busController)
{
	if(!busController)
	{
		throw std::runtime_error("BusController passed for registration is null");
	}

	if(!instance)
	{
		instance = busController;
		return true;
	}
	return false;
}

bool BusController::HasBusController()
{
	return (instance!=nullptr);
}

void BusController::initializeBusController(std::set<Entity*>& agentList)
{
	const ConfigParams& configParams = ConfigManager::GetInstance().FullConfig();
	vector<PT_BusDispatchFreq> dispatchFreq;

	DbLoader dataLoader(configParams.getDatabaseConnectionString(false));

	const std::map<std::string, std::string>& storedProcs = configParams.getDatabaseProcMappings().procedureMappings;

	std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("pt_bus_dispatch_freq");
	if(spIt == storedProcs.end())
	{
		throw std::runtime_error("missing stored procedure for pt_bus_dispatch_freq");
	}
	dataLoader.loadPTBusDispatchFreq(spIt->second, dispatchFreq);

	spIt = storedProcs.find("pt_bus_routes");
	if(spIt == storedProcs.end())
	{
		throw std::runtime_error("missing stored procedure for pt_bus_routes");
	}
	dataLoader.loadPTBusRoutes(spIt->second, busRouteMap);

	spIt = storedProcs.find("pt_bus_stops");
	if(spIt == storedProcs.end())
	{
		throw std::runtime_error("missing stored procedure for pt_bus_stops");
	}
	dataLoader.loadPTBusStops(spIt->second, busStopSequenceMap, busRouteMap);

	setPTScheduleFromConfig(dispatchFreq);
	assignBusTripChainWithPerson(agentList);

	//initialize bus route links map for passenger route choice
	busRouteLinksMap.clear();
	for(std::map<std::string, std::vector<const RoadSegment*> >::const_iterator routeMapIt = busRouteMap.begin();
			routeMapIt != busRouteMap.end(); routeMapIt++)
	{
		const string& busline = routeMapIt->first;
		const std::vector<const RoadSegment*>& segRoute = routeMapIt->second;
		std::vector<const Link*>& linkRoute = busRouteLinksMap[busline];
		const Link* currLink = nullptr;
		for(const RoadSegment* seg : segRoute)
		{
			if(currLink != seg->getParentLink())
			{
				currLink = seg->getParentLink();
				linkRoute.push_back(currLink);
			}
		}
	}
}

BusController* BusController::GetInstance()
{
	return instance;
}

std::vector<BufferedBase *> BusController::buildSubscriptionList()
{
	return Agent::buildSubscriptionList();
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
	unsigned int linkId;
	int segmentIndex;
};

struct StopInfo
{
	std::string line;
	std::string id;
	unsigned int posX;
	unsigned int posY;
	int index;
};

bool searchBusRoutes(const vector<const BusStop*>& stops, const std::string& busLine, std::deque<RouteInfo>& allRoutes, std::deque<StopInfo>& allStops)
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
				stopInfo.id = start->getStopCode();
				stopInfo.line = busLine;
				stopInfo.posX = start->getStopLocation().getX();
				stopInfo.posY = start->getStopLocation().getY();
				stopIDs.push_back(stopInfo);
			}
			else
			{
				end = busStop;
				const StreetDirectory& stdir = StreetDirectory::Instance();
				vector<WayPoint> path;
				if (start->getRoadSegmentId() == end->getRoadSegmentId())
				{
					path.push_back(WayPoint(start->getParentSegment()));
				}
				else
				{
					const A_StarShortestPathImpl* shortestDir = (A_StarShortestPathImpl*)(stdir.getDistanceImpl());
					path = shortestDir->SearchShortestDrivingPath<RoadSegment>(*start, *end);
				}

				for (std::vector<WayPoint>::const_iterator it = path.begin();
						it != path.end(); it++)
				{
					if (it->type == WayPoint::ROAD_SEGMENT)
					{
						unsigned int id = (*it).roadSegment->getRoadSegmentId();
						const Link* link = (*it).roadSegment->getParentLink();
						if (routeIDs.size() == 0 || routeIDs.back().id != id)
						{
							RouteInfo route;
							route.id = id;
							route.start = start->getStopCode();
							route.end = end->getStopCode();
							route.startPosX = start->getStopLocation().getX();
							route.startPosY = start->getStopLocation().getY();
							route.endPosX = end->getStopLocation().getX();
							route.endPosY = end->getStopLocation().getY();
							route.linkId = link->getLinkId();
							route.segmentIndex = (*it).roadSegment->getSequenceNumber();
							routeIDs.push_back(route);
						}
						isFound = true;
					}
				}

					if (!isFound)
					{
						std::cout << "can not find bus route in bus line:" << busLine
								<< " start stop:" << start->getStopCode()
								<< "  end stop:" << end->getStopCode() << std::endl;
						routeIDs.clear();
						stopIDs.clear();
						break;
					}
				else
				{
					StopInfo stopInfo;
					stopInfo.id =  end->getStopCode();
					stopInfo.line = busLine;
					stopInfo.posX = end->getStopLocation().getX();
					stopInfo.posY = end->getStopLocation().getY();
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
				routeInfo.linkId = it->linkId;
				routeInfo.segmentIndex = it->segmentIndex;
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

void BusController::setPTScheduleFromConfig(const vector<PT_BusDispatchFreq>& dispatchFreq)
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	vector<const BusStop*> stops;
	BusLine* busline = nullptr;
	int step = 0;
	busDrivers.clear();
	bool busLineRegistered=false;
	DailyTime lastBusDispatchTime;
	std::deque<RouteInfo> allRoutes;
	std::deque<StopInfo> allStops;

	for (vector<PT_BusDispatchFreq>::const_iterator curr=dispatchFreq.begin(); curr!=dispatchFreq.end(); curr++)
	{
		vector<PT_BusDispatchFreq>::const_iterator next = curr+1;

		//If we're on a new BusLine, register it with the scheduler.
		if(!busline || (curr->routeId != busline->getBusLineID()))
		{
			busline = new BusLine(curr->routeId,config.busController.busLineControlType);
			ptSchedule.registerBusLine(curr->routeId, busline);
			ptSchedule.registerControlType(curr->routeId, busline->getControlType());
			step = 0;
			busLineRegistered = true;
		}

		// define frequency_busline for one busline
		busline->addBusLineFrequency(BusLineFrequency(curr->startTime, curr->endTime, curr->headwaySec));

		//Set nextTime to the next frequency bus line's start time or the current line's end time if this is the last line.
		DailyTime nextTime = curr->endTime;

		DailyTime advance(curr->headwaySec*MS_IN_UNIT_SEC);
		DailyTime startTime = curr->startTime;
		for(; startTime.isBeforeEqual(nextTime); startTime += advance)
		{
			// deal with small gaps between the group dispatching times
			if ((startTime - lastBusDispatchTime).isBeforeEqual(advance))
			{
				startTime = lastBusDispatchTime + advance;
			}

			BusTrip bustrip("", "BusTrip", 0, -1, startTime, DailyTime("00:00:00"), step++, busline, -1, curr->routeId, nullptr, "node", nullptr, "node");

			//Try to find our data.
			vector<const RoadSegment*> segments = vector<const RoadSegment*>();
			map<string, vector<const RoadSegment*> >::const_iterator itSeg = busRouteMap.find(curr->routeId);
			if (itSeg != busRouteMap.end())
			{
				segments = itSeg->second;
			}

			stops = vector<const BusStop*>();
			map<string, vector<const BusStop*> >::const_iterator itStop = busStopSequenceMap.find(curr->routeId);
			if (itStop != busStopSequenceMap.end())
			{
				stops = itStop->second;
			}

			if (busLineRegistered && ConfigManager::GetInstance().FullConfig().generateBusRoutes)
			{
				searchBusRoutes(stops, curr->routeId, allRoutes,allStops);
			}

			if (bustrip.setBusRouteInfo(segments, stops))
			{
				busline->addBusTrip(bustrip);
			}

			lastBusDispatchTime = startTime;
		}
	}

	if (ConfigManager::GetInstance().FullConfig().generateBusRoutes)
	{
		std::ofstream outputRoutes("routes.csv");
		for (std::deque<RouteInfo>::const_iterator it = allRoutes.begin(); it != allRoutes.end(); it++)
		{
			if (outputRoutes.is_open())
			{
				outputRoutes << it->line << "," << it->index << "," << it->id << std::endl;
			}
		}
		outputRoutes.close();

		std::ofstream outputStop("stops.csv");
		for (std::deque<StopInfo>::const_iterator it = allStops.begin(); it != allStops.end(); it++)
		{
			if (outputStop.is_open())
			{
				outputStop << it->line << "," << it->id << "," << it->index << std::endl;
			}
		}
		outputStop.close();
	}
}

void BusController::storeRealTimesAtEachBusStop(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, const BusStop* lastVisitedBusStop, BusStopRealTimes& realTime)
{
	BusLine* busline = ptSchedule.findBusLine(busLine);
	if(!busline) {
		return;
	}

	double departureTime = arrivalTime + (departTime * MS_IN_UNIT_SEC);
	BusStopRealTimes busStopRealTimes(ConfigManager::GetInstance().FullConfig().simStartTime() + DailyTime(arrivalTime), ConfigManager::GetInstance().FullConfig().simStartTime() + DailyTime(departureTime));
	busStopRealTimes.setRealBusStop(lastVisitedBusStop);
	realTime = (busStopRealTimes);

	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	Shared<BusStopRealTimes>* sharedStopRealTimes = new Shared<BusStopRealTimes>(config.mutexStategy(), busStopRealTimes);
	// set this value for next step
	busline->resetBusTripStopRealTimes(trip, sequence, sharedStopRealTimes);
}

double BusController::computeDwellTime(const string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisitedBusStop)
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
		waitTimeBusStop = (departureTime - arrivalTime) * SECS_IN_UNIT_MS;
		break;
	case HEADWAY_BASED:
		departureTime = headwayDecision(busLine, trip, sequence, arrivalTime, departTime, realTime, lastVisitedBusStop);
		waitTimeBusStop = (departureTime - arrivalTime) * SECS_IN_UNIT_MS;
		break;
	case EVENHEADWAY_BASED:
		departureTime = evenheadwayDecision(busLine, trip, sequence, arrivalTime, departTime, realTime, lastVisitedBusStop);
		waitTimeBusStop = (departureTime - arrivalTime) * SECS_IN_UNIT_MS;
		break;
	case HYBRID_BASED:
		departureTime = hybridDecision(busLine, trip, sequence, arrivalTime, departTime, realTime, lastVisitedBusStop);
		waitTimeBusStop = (departureTime - arrivalTime) * SECS_IN_UNIT_MS;
		break;
	default:
		storeRealTimesAtEachBusStop(busLine, trip, sequence, arrivalTime, departTime, lastVisitedBusStop, realTime);
		waitTimeBusStop = departTime;
		break;
	}
	return waitTimeBusStop;
}

double BusController::scheduledDecision(const string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisitedBusStop)
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
	estimatedTime = std::max(assumedTime, arrivalTime + (departTime * MS_IN_UNIT_SEC));

	storeRealTimesAtEachBusStop(busLine, trip, sequence, arrivalTime, departTime, lastVisitedBusStop, realTime);

	return estimatedTime;
}

double BusController::headwayDecision(const string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisitedBusStop)
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
		estimatedTime = std::max(arrivalTimeMinusOne + alpha * PLANNED_HEADWAY_MS, arrivalTime + (departTime * MS_IN_UNIT_SEC));
	} else {
		// immediately leave
		estimatedTime = arrivalTime + (departTime * MS_IN_UNIT_SEC);
	}

	storeRealTimesAtEachBusStop(busLine, trip, sequence, arrivalTime, departTime, lastVisitedBusStop, realTime);

	return estimatedTime;
}

double BusController::evenheadwayDecision(const string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisitedBusStop)
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
		estimatedTime = arrivalTime + (departTime * MS_IN_UNIT_SEC);

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
				arrivalTime + (departTime * MS_IN_UNIT_SEC));
	}

	storeRealTimesAtEachBusStop(busLine, trip, sequence, arrivalTime, departTime, lastVisitedBusStop, realTime);

	return estimatedTime;
}

double BusController::hybridDecision(const string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisitedBusStop)
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
		estimatedTime = arrivalTime + (departTime * MS_IN_UNIT_SEC);

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
				(double) (arrivalTime) + (departTime * MS_IN_UNIT_SEC));
	}

	storeRealTimesAtEachBusStop(busLine, trip, sequence, arrivalTime, departTime, lastVisitedBusStop, realTime);

	return estimatedTime;
}

void BusController::addOrStashBuses(Person* p, std::set<Entity*>& activeAgents)
{
	if (p->getStartTime()==0) {
		//Only agents with a start time of zero should start immediately in the all_agents list.
		activeAgents.insert(p);
	} else {
		//Start later.
		pendingChildren.push(p);
	}
}

void BusController::unregisterChild(Entity* child)
{
	if (child)
	{
		std::vector<Entity*>::iterator it = std::find(busDrivers.begin(), busDrivers.end(), child);
		if (it != busDrivers.end())
		{
			busDrivers.erase(it);
		}
	}
}

Entity::UpdateStatus BusController::frame_tick(timeslice now)
{
	nextTimeTickToStage++;
	unsigned int nextTickMS = nextTimeTickToStage * ConfigManager::GetInstance().FullConfig().baseGranMS();

	//Stage any pending entities that will start during this time tick.
	while (!pendingChildren.empty() && pendingChildren.top()->getStartTime() <= nextTickMS)
	{
		//Ask the current worker's parent WorkGroup to schedule this Entity.
		Entity* child = pendingChildren.top();
		pendingChildren.pop();
		child->parentEntity = this;
		Person *per=dynamic_cast<Person*>(child);
		currWorkerProvider->scheduleForBred(child);
		busDrivers.push_back(child);
	}

	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus BusController::frame_init(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}

void BusController::frame_output(timeslice now)
{
}

void BusController::load(const std::map<std::string, std::string>& configProps)
{
}

bool BusController::isNonspatial()
{
	return true;
}

const std::vector<const Link*>& sim_mob::BusController::getLinkRoute(const std::string& busline) const
{
	if(busline.empty())
	{
		throw std::runtime_error("empty busline passed for fetching link route");
	}
	std::map<std::string, std::vector<const Link*> >::const_iterator lnkRouteMapIt = busRouteLinksMap.find(busline);
	if(lnkRouteMapIt == busRouteLinksMap.end())
	{
		throw std::runtime_error("invalid busline passed for fetching link route: " + busline);
	}
	return lnkRouteMapIt->second;
}

const std::vector<const BusStop*>& sim_mob::BusController::getStops(const std::string& busline) const
{
	if(busline.empty())
	{
		throw std::runtime_error("empty busline passed for fetching stop list");
	}
	std::map<std::string, std::vector<const BusStop*> >::const_iterator stopsMapIt = busStopSequenceMap.find(busline);
	if(stopsMapIt == busStopSequenceMap.end())
	{
		throw std::runtime_error("invalid busline passed for fetching stops list: " + busline);
	}
	return stopsMapIt->second;
}

bool sim_mob::BusController::isBuslineAvailable(const std::vector<std::string>& busLineIds, const DailyTime& time) const
{
	if(busLineIds.empty())
	{
		throw std::runtime_error("empty busline ids vector passed for fetching availability");
	}
	bool busesAvailable = false;
	const BusLine* busline;
	for(const std::string& line : busLineIds)
	{
		busline = ptSchedule.findBusLine(line);
		if(busline)
		{
			busesAvailable = (busesAvailable || busline->isAvailable(time));
		}
		else
		{
			std::stringstream msg; msg <<"invalid busline passed for fetching availability (" << line << ")";
		}
	}
	return busesAvailable;
}
