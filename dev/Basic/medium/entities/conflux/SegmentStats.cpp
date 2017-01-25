//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "SegmentStats.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "config/MT_Config.hpp"
#include "Conflux.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/roles/Role.hpp"
#include "entities/vehicle/VehicleBase.hpp"
#include "logging/Log.hpp"
#include "message/MessageBus.hpp"
#include "entities/TaxiStandAgent.hpp"
#include "entities/roles/driver/TaxiDriverFacets.hpp"

using std::string;
using namespace sim_mob;
using namespace sim_mob::medium;

namespace
{
#define GET_LANE_INFINITY_ID(X) (X*100+9)

const double INFINITESIMAL_DOUBLE = 0.000001;
const double SHORT_SEGMENT_LENGTH_LIMIT = 5 * PASSENGER_CAR_UNIT; // 5 times a car's length (in m)
const double LARGE_OUTPUT_FLOW_RATE = 2.77; //veh/s (10000 veh/hr)(considered high output flow rate for a lane) (suggested by Yang Lu on 23-Apr-2015)

const double SINGLE_LANE_SEGMENT_CAPACITY = 1200.0; //veh/hr. suggested by Yang Lu on 11-Oct-2014
const double DOUBLE_LANE_SEGMENT_CAPACITY = 3000.0; //veh/hr. suggested by Yang Lu on 11-Oct-2014
}

bool GreaterDistToSegmentEnd::operator ()(const Person_MT* x, const Person_MT* y) const
{
	if ((!x) || (!y))
	{
		std::stringstream debugMsgs;
		debugMsgs << "cmp_person_remainingTimeThisTick: Comparison failed because at least one of the arguments is null" << "|x: " << (x ? x->getId() : 0)
				<< "|y: " << (y ? y->getId() : 0);
		throw std::runtime_error(debugMsgs.str());
	}
	//person x > y iff x's distance to end of segment is greater than y's
	return (x->distanceToEndOfSegment > y->distanceToEndOfSegment);
}

///*
// * The parameters - min density, jam density, alpha and beta -
// * must be obtained for each road segment from an external source (XML/Database)
// * Since we don't have this data, we have taken the average values from
// * supply parameters of Singapore expressways.
// *
// * TODO: This must be changed when we have this information for each
// * road segment in the network.
// */
//SupplyParams::SupplyParams(const RoadSegment* rdSeg, double statsLength) :
//		freeFlowSpeed(convertKmphToCmps(rdSeg->maxSpeed)),
//		minSpeed(0.3 * freeFlowSpeed), /*30% of free flow speed as suggested by Yang Lu*/
//		jamDensity(0.2), /*density during traffic jam in veh/meter*/
//		minDensity(0.0048), /*minimum traffic density in veh/meter*/
//		capacity(rdSeg->getCapacity() / 3600.0), /*converting capacity to vehicles/hr to vehicles/s*/
//		alpha(1.8), beta(1.9)
//{}

SupplyParams::SupplyParams(const RoadSegment* rdSeg, double statsLength) :
		freeFlowSpeed(rdSeg->getMaxSpeed()), minSpeed(0.2 * freeFlowSpeed), /*20% of free flow speed as suggested by Yang Lu*/
		jamDensity(0.2), minDensity(0.2 * 0.2), capacity(rdSeg->getCapacity()),
		alpha(0.0),	beta(0.0)
{
	int linkCategory = static_cast<int>(rdSeg->getParentLink()->getLinkCategory());
	SpeedDensityParams speedDensityParams = MT_Config::getInstance().getSpeedDensityParam(linkCategory);
	alpha = speedDensityParams.getAlpha();
	beta = speedDensityParams.getBeta();
	jamDensity = speedDensityParams.getJamDensity();
	minDensity = speedDensityParams.getMinDensity();

	//TESTING ~ Harish
//	unsigned int numLanes = rdSeg->getLanes().size();
//	capacity = capacity + (0.42 * numLanes); // increase by 1600 veh/hr
//	freeFlowSpeed = freeFlowSpeed + 5.55556; //increase by 20kmph
//	minSpeed = 0.2 * freeFlowSpeed;
}

SegmentStats::SegmentStats(const RoadSegment* rdSeg, Conflux* parentConflux, double statslengthInM) :
		roadSegment(rdSeg), length(statslengthInM), segDensity(0.0), segFlow(0), numPersons(0), statsNumberInSegment(1), supplyParams(rdSeg, statslengthInM), orderBySetting(
				SEGMENT_ORDERING_BY_DISTANCE_TO_INTERSECTION), parentConflux(parentConflux)
{
	segVehicleSpeed = roadSegment->getMaxSpeed();
	numVehicleLanes = 0;

	// initialize LaneAgents in the map
	std::vector<Lane*>::const_iterator laneIt = rdSeg->getLanes().begin();
	while (laneIt != rdSeg->getLanes().end())
	{
		LaneStats* lnStats = new LaneStats(*laneIt, length);
		laneStatsMap.insert(std::make_pair(*laneIt, lnStats));
		lnStats->initLaneParams(segVehicleSpeed, supplyParams.getCapacity());
		if (!(*laneIt)->isPedestrianLane())
		{
			numVehicleLanes++;
			outermostLane = *laneIt;
		}
		lnStats->setParentStats(this);
		laneIt++;
	}

	/*
	 * Any lane with an id ending with 9 is laneInfinity of the road segment.
	 * This lane is available only to the SegmentStats and not the parent RoadSegment.
	 * TODO: Must check if we can have a bit pattern (Refer lane constructor) for laneInfinity.
	 */
	laneInfinity = new Lane();//const_cast<RoadSegment*>(rdSeg), 9);
	laneInfinity->setLaneId(GET_LANE_INFINITY_ID(rdSeg->getRoadSegmentId()));
	laneInfinity->setBusLaneRules((sim_mob::BusLaneRules) 0);
	laneInfinity->setCanVehiclePark(0);
	laneInfinity->setCanVehicleStop(0);
	laneInfinity->setHasRoadShoulder(0);
	laneInfinity->setHighOccupancyVehicleAllowed(true);
	laneInfinity->setRoadSegmentId(rdSeg->getRoadSegmentId());
	laneInfinity->setParentSegment(const_cast<RoadSegment*>(rdSeg));
	laneInfinity->setWidth(0);
	LaneStats* lnInfStats = new LaneStats(laneInfinity, statslengthInM, true);
	laneStatsMap.insert(std::make_pair(laneInfinity, lnInfStats));
	lnInfStats->setParentStats(this);
}

SegmentStats::~SegmentStats()
{
	for (LaneStatsMap::iterator i = laneStatsMap.begin(); i != laneStatsMap.end(); i++)
	{
		safe_delete_item(i->second);
	}
	for (BusStopAgentList::iterator i = busStopAgents.begin(); i != busStopAgents.end(); i++)
	{
		(*i)->currWorkerProvider = nullptr;
		safe_delete_item(*i);
	}
	safe_delete_item(laneInfinity);
}

void SegmentStats::updateBusStopAgents(timeslice now)
{
	for (BusStopAgentList::iterator i = busStopAgents.begin(); i != busStopAgents.end(); i++)
	{
		(*i)->update(now);
	}

	for(auto it = taxiStandAgents.begin(); it != taxiStandAgents.end(); it++)
	{
		(*it)->update(now);
	}
}

void SegmentStats::addAgent(const Lane* lane, Person_MT* p)
{
	boost::unique_lock<boost::recursive_mutex> lock(mutexPersonManagement);
	laneStatsMap.find(lane)->second->addPerson(p);
	numPersons++; //record addition to segment
}

bool SegmentStats::removeAgent(const Lane* lane, Person_MT* p, bool wasQueuing, double vehicleLength)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::removeAgent lane not found in segment stats");
	}
	bool removed = laneIt->second->removePerson(p, wasQueuing, vehicleLength);
	if (removed)
	{
		numPersons--;
	} //record removal from segment
	return removed;
}

void SegmentStats::updateQueueStatus(const Lane* lane, Person_MT* p)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		std::stringstream out("");
		out << "SegmentStats::updateQueueStatus lane not found in segment stats. Segment[" << roadSegment->getRoadSegmentId() << "] index" << statsNumberInSegment;
		throw std::runtime_error(out.str());
	}
	laneIt->second->updateQueueStatus(p);
}

std::deque<Person_MT*>& SegmentStats::getPersons(const Lane* lane)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::getPersons lane not found in segment stats");
	}
	return laneIt->second->laneAgents;
}

std::vector<const BusStop*>& SegmentStats::getBusStops()
{
	return busStops;
}

void SegmentStats::addBusStopAgent(BusStopAgent* busStopAgent)
{
	if (!busStopAgent)
	{
		return;
	}
	busStopAgents.push_back(busStopAgent);
}

void SegmentStats::addTaxiStandAgent(TaxiStandAgent* taxiStandAgent)
{
	if(!taxiStandAgent)
	{
		return;
	}
	taxiStandAgents.push_back(taxiStandAgent);
}

void SegmentStats::initializeBusStops()
{
	for (BusStopAgentList::iterator stopAgIt = busStopAgents.begin(); stopAgIt != busStopAgents.end(); stopAgIt++)
	{
		Agent* stopAgent = *stopAgIt;
		if (!stopAgent->isInitialized())
		{
			messaging::MessageBus::RegisterHandler(stopAgent);
			stopAgent->setInitialized(true);
		}
	}
}

void SegmentStats::addBusDriverToStop(Person_MT* driver, const BusStop* stop)
{
	if (stop && hasBusStop(stop))
	{
		busDrivers.at(stop).push_back(driver);
	}
}

void SegmentStats::removeBusDriverFromStop(Person_MT* driver, const BusStop* stop)
{
	if (stop && hasBusStop(stop))
	{
		PersonList& driversAtStop = busDrivers.at(stop);
		PersonList::iterator driverIt = std::find(driversAtStop.begin(), driversAtStop.end(), driver);
		if (driverIt != driversAtStop.end())
		{
			driversAtStop.erase(driverIt);
		}
		else
		{
			throw std::runtime_error("attempt to remove a bus driver who is not serving the stop");
		}
	}
	else
	{
		throw std::runtime_error("Bus stop not found in SegmentStats");
	}
}

void SegmentStats::getPersons(std::deque<Person_MT*>& segAgents)
{
	for (LaneStatsMap::iterator lnStMpIt = laneStatsMap.begin(); lnStMpIt != laneStatsMap.end(); lnStMpIt++)
	{
		PersonList& lnAgents = lnStMpIt->second->laneAgents;
		segAgents.insert(segAgents.end(), lnAgents.begin(), lnAgents.end());
	}

	for (BusStopList::const_reverse_iterator stopIt = busStops.rbegin(); stopIt != busStops.rend(); stopIt++)
	{
		const BusStop* stop = *stopIt;
		PersonList& driversAtStop = busDrivers.at(stop);
		segAgents.insert(segAgents.end(), driversAtStop.begin(), driversAtStop.end());
	}
}

void SegmentStats::getInfinityPersons(std::deque<Person_MT*>& segAgents)
{
	PersonList& lnAgents = laneStatsMap.find(laneInfinity)->second->laneAgents;
	segAgents.insert(segAgents.end(), lnAgents.begin(), lnAgents.end());
}

void SegmentStats::topCMergeLanesInSegment(PersonList& mergedPersonList)
{
	mergedPersonList.clear();
	//Bus drivers go in the front of the list, because bus stops are (virtually) located at the end of the segment
	for (BusStopList::const_reverse_iterator stopIt = busStops.rbegin(); stopIt != busStops.rend(); stopIt++)
	{
		const BusStop* stop = *stopIt;
		PersonList& driversAtStop = busDrivers.at(stop);
		for (PersonList::iterator pIt = driversAtStop.begin(); pIt != driversAtStop.end(); pIt++)
		{
			mergedPersonList.push_back(*pIt);
		}
	}

	int capacity = (int) (ceil(supplyParams.getCapacity()));
	//init iterator list to the front of each lane
	std::vector<PersonList::iterator> iteratorLists;
	for (LaneStatsMap::iterator lnIt = laneStatsMap.begin(); lnIt != laneStatsMap.end(); lnIt++)
	{
		if(!lnIt->second->isLaneInfinity())
		{
			iteratorLists.push_back(lnIt->second->laneAgents.begin());
		}
	}

	//pick the Top C
	for (int c = 0; c < capacity; c++)
	{
		double minVal = std::numeric_limits<double>::max();
		Person_MT* currPerson = nullptr;
		std::vector<std::pair<int, Person_MT*> > equiDistantList;
		int i = 0;
		for (LaneStatsMap::iterator lnIt = laneStatsMap.begin(); lnIt != laneStatsMap.end(); lnIt++)
		{
			if(!lnIt->second->isLaneInfinity())
			{
				PersonList& personsInLane = lnIt->second->laneAgents;
				if (iteratorLists[i] != personsInLane.end())
				{
					currPerson = (*(iteratorLists[i]));
					if (orderBySetting == SEGMENT_ORDERING_BY_DISTANCE_TO_INTERSECTION)
					{
						if (currPerson->distanceToEndOfSegment == minVal)
						{
							equiDistantList.push_back(std::make_pair(i, currPerson));
						}
						else if (currPerson->distanceToEndOfSegment < minVal)
						{
							minVal = currPerson->distanceToEndOfSegment;
							equiDistantList.clear();
							equiDistantList.push_back(std::make_pair(i, currPerson));
						}
					}
					else if (orderBySetting == SEGMENT_ORDERING_BY_DRIVING_TIME_TO_INTERSECTION)
					{
						if (currPerson->drivingTimeToEndOfLink == minVal)
						{
							equiDistantList.push_back(std::make_pair(i, currPerson));
						}
						else if (currPerson->drivingTimeToEndOfLink < minVal)
						{
							minVal = currPerson->drivingTimeToEndOfLink;
							equiDistantList.clear();
							equiDistantList.push_back(std::make_pair(i, currPerson));
						}
					}
				}
				i++;
			}
		}

		if (!equiDistantList.empty())
		{
			//we have to randomly choose from persons in equiDistantList
			size_t numElements = equiDistantList.size();
			std::pair<int, Person_MT*> chosenPair;
			if (numElements == 1)
			{
				chosenPair = equiDistantList.front();
			}
			else
			{
				int chosenIdx = rand() % numElements;
				chosenPair = equiDistantList[chosenIdx];
			}
			iteratorLists.at(chosenPair.first)++;
			mergedPersonList.push_back(chosenPair.second);
		}
	}

	//After picking the Top C, just append the remaining vehicles in the output list
	int i = 0;
	for (LaneStatsMap::iterator lnIt = laneStatsMap.begin(); lnIt != laneStatsMap.end(); lnIt++)
	{
		if(!lnIt->second->isLaneInfinity())
		{
			PersonList& personsInLane = lnIt->second->laneAgents;
			if (iteratorLists[i] != personsInLane.end())
			{
				mergedPersonList.insert(mergedPersonList.end(), iteratorLists[i], personsInLane.end());
			}
			i++;
		}
	}

	//insert lane infinity persons at the tail of mergedPersonList
	LaneStats* lnInfStats =  laneStatsMap[laneInfinity];
	mergedPersonList.insert(mergedPersonList.end(), lnInfStats->laneAgents.begin(), lnInfStats->laneAgents.end());
}

std::pair<unsigned int, unsigned int> SegmentStats::getLaneAgentCounts(const Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::getLaneAgentCounts lane not found in segment stats");
	}
	return std::make_pair(laneIt->second->getQueuingAgentsCount(), laneIt->second->getMovingAgentsCount());
}

double SegmentStats::getLaneQueueLength(const Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::getLaneQueueLength lane not found in segment stats");
	}
	return laneIt->second->getQueueLength();
}

double SegmentStats::getLaneMovingLength(const Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::getLaneMovingLength lane not found in segment stats");
	}
	return laneIt->second->getMovingLength();
}

double SegmentStats::getLaneTotalVehicleLength(const Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::getLaneTotalVehicleLength lane not found in segment stats");
	}
	return laneIt->second->getTotalVehicleLength();
}

unsigned int SegmentStats::numAgentsInLane(const Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::numAgentsInLane lane not found in segment stats");
	}
	return laneIt->second->getNumPersons();
}

unsigned int SegmentStats::numMovingInSegment(bool hasVehicle) const
{
	unsigned int movingCounts = 0;
	const std::vector<Lane*>& segLanes = roadSegment->getLanes();
	std::vector<Lane*>::const_iterator laneIt = segLanes.begin();
	while (laneIt != segLanes.end())
	{
		if ((hasVehicle && !(*laneIt)->isPedestrianLane()) || (!hasVehicle && (*laneIt)->isPedestrianLane()))
		{
			LaneStatsMap::const_iterator laneStatsIt = laneStatsMap.find(*laneIt);
			if (laneStatsIt != laneStatsMap.end())
			{
				movingCounts = movingCounts + laneStatsIt->second->getMovingAgentsCount();
			}
			else
			{
				throw std::runtime_error("SegmentStats::numMovingInSegment called with invalid laneStats.");
			}
		}
		laneIt++;
	}
	return movingCounts;
}

double SegmentStats::getMovingLength() const
{
	double movingLength = 0;
	for (LaneStatsMap::const_iterator laneStatsIt = laneStatsMap.begin(); laneStatsIt != laneStatsMap.end(); laneStatsIt++)
	{
		if (!laneStatsIt->second->isLaneInfinity() && !laneStatsIt->first->isPedestrianLane())
		{
			movingLength = movingLength + laneStatsIt->second->getMovingLength();
		}
	}
	return movingLength;
}

double SegmentStats::getQueueLength() const
{
	double queueLength = 0;
	for (LaneStatsMap::const_iterator laneStatsIt = laneStatsMap.begin(); laneStatsIt != laneStatsMap.end(); laneStatsIt++)
	{
		if (!laneStatsIt->second->isLaneInfinity() && !laneStatsIt->first->isPedestrianLane())
		{
			queueLength = queueLength + laneStatsIt->second->getQueueLength();
		}
	}
	return queueLength;
}

bool SegmentStats::hasQueue() const
{
	for (LaneStatsMap::const_iterator laneStatsIt = laneStatsMap.begin(); laneStatsIt != laneStatsMap.end(); laneStatsIt++)
	{
		if (!laneStatsIt->second->isLaneInfinity()
				&& !laneStatsIt->first->isPedestrianLane()
				&& laneStatsIt->second->getQueueLength() > 0.0)
		{
			return true;
		}
	}
	return false;
}

double SegmentStats::getTotalVehicleLength() const
{
	double totalLength = 0;
	for (LaneStatsMap::const_iterator laneStatsIt = laneStatsMap.begin(); laneStatsIt != laneStatsMap.end(); laneStatsIt++)
	{
		if (!laneStatsIt->second->isLaneInfinity() && !laneStatsIt->first->isPedestrianLane())
		{
			totalLength = totalLength + laneStatsIt->second->getTotalVehicleLength();
		}
	}
	return totalLength;
}

//density will be computed in vehicles/meter-lane for the moving part of the segment
double SegmentStats::getDensity(bool hasVehicle)
{
	double density = 0.0;
	double queueLength = getQueueLength();
	double movingPartLength = length * numVehicleLanes - queueLength;
	double movingPCUs = getMovingLength() / PASSENGER_CAR_UNIT;
	if(movingPCUs>0&&getRoadSegment()->getRoadSegmentId()==5414)
	{
		int debug =1 ;
	}
	if (movingPartLength > PASSENGER_CAR_UNIT)
	{
		density = movingPCUs / movingPartLength;
	}
	else
	{
		density = 1 / PASSENGER_CAR_UNIT;
	}
	return density;
}

//density will be computed in vehicles/lane-km for the full segment
double SegmentStats::getTotalDensity(bool hasVehicle)
{
	double density = 0.0;
	double totalPCUs = getTotalVehicleLength() / PASSENGER_CAR_UNIT;
	density = totalPCUs / (numVehicleLanes * (length / 1000.0));

	return density;
}

unsigned int SegmentStats::numQueuingInSegment(bool hasVehicle) const
{
	unsigned int queuingCounts = 0;
	const std::vector<Lane*>& segLanes = roadSegment->getLanes();
	std::vector<Lane*>::const_iterator lane = segLanes.begin();
	while (lane != segLanes.end())
	{
		if ((hasVehicle && !(*lane)->isPedestrianLane()) || (!hasVehicle && (*lane)->isPedestrianLane()))
		{
			LaneStatsMap::const_iterator laneStatsIt = laneStatsMap.find(*lane);
			if (laneStatsIt != laneStatsMap.end())
			{
				queuingCounts = queuingCounts + laneStatsIt->second->getQueuingAgentsCount();
			}
			else
			{
				throw std::runtime_error("SegmentStats::numQueueingInSegment was called with invalid laneStats!");
			}
		}
		lane++;
	}
	return queuingCounts;
}

void SegmentStats::addBusStop(const BusStop* stop)
{
	if (stop)
	{
		busStops.push_back(stop);
		busDrivers[stop] = PersonList();
	}
	else
	{
		throw std::runtime_error("addBusStop(): stop to be added is NULL");
	}
}

void SegmentStats::addTaxiStand(const TaxiStand* stand)
{
	if(stand)
	{
		taxiStands.push_back(stand);
	}
}

std::vector<const TaxiStand*>& SegmentStats::getTaxiStand()
{
	return taxiStands;
}

unsigned int LaneStats::getQueuingAgentsCount() const
{
	return queueCount;
}

unsigned int LaneStats::getMovingAgentsCount() const
{
	if (numPersons < queueCount)
	{
		printAgents();
		std::stringstream debugMsgs;
		debugMsgs << "number of lane agents cannot be less than the number of queuing agents." << "\nlane" << getLane()->getLaneId() << "|queueCount: "
				<< queueCount << "|laneAgents count: " << numPersons << std::endl;
		throw std::runtime_error(debugMsgs.str());
	}
	return (numPersons - queueCount);
}

double LaneStats::getMovingLength() const
{
	if (totalLength < queueLength)
	{
		printAgents();
		std::stringstream debugMsgs;
		debugMsgs << "totalLength cannot be less than queueLength." << "\nlane" << getLane()->getLaneId() << "|queueLength: " << queueLength << "|totalLength: "
				<< totalLength << std::endl;
		throw std::runtime_error(debugMsgs.str());
	}
	return (totalLength - queueLength);
}

void LaneStats::addPerson(Person_MT* p)
{
	VehicleBase* vehicle = nullptr;
	if (laneInfinity)
	{
		laneAgents.push_back(p);
		numPersons++;
	}
	else
	{
		vehicle = p->getRole()->getResource(); //person will surely have a role if he is getting added to any lane which is not lane infinity
		if (laneAgents.size() > 0)
		{
			std::deque<Person_MT*>::iterator i = laneAgents.end() - 1; // last person's iterator
			while (i != laneAgents.begin() && (*i)->distanceToEndOfSegment > p->distanceToEndOfSegment)
			{
				i--;
			}
			if (i == laneAgents.begin() && (*i)->distanceToEndOfSegment > p->distanceToEndOfSegment)
			{
				laneAgents.push_front(p);
			}
			else
			{
				laneAgents.insert(i + 1, p); //deque is optimized for insertions and removals.
			}
		}
		else
		{
			laneAgents.push_back(p);
		}
		if (vehicle)
		{
			numPersons++; // record addition
			totalLength = totalLength + vehicle->getLengthInM();
			if (p->isQueuing)
			{
				queueCount++;
				queueLength = queueLength + vehicle->getLengthInM();
			}
		}
		else
		{
			throw std::runtime_error("Person with no vehicle is added to lane");
		}
		//verifyOrdering();
	}
}

void LaneStats::updateQueueStatus(Person_MT* p)
{
	VehicleBase* vehicle = p->getRole()->getResource();
	if (!laneInfinity && vehicle)
	{
		if (p->isQueuing)
		{
			queueCount++;
			queueLength = queueLength + vehicle->getLengthInM();
		}
		else
		{
			if (queueCount > 0)
			{
				queueCount--;
				queueLength = queueLength - vehicle->getLengthInM();
			}
			else
			{
				std::stringstream debugMsgs;
				debugMsgs << "Error in updateQueueStatus(): queueCount cannot be lesser than 0 in lane." << "\nlane:" << lane->getLaneId() << "|Segment: "
						<< lane->getParentSegment()->getRoadSegmentId() << "|Person: " << p->getId() << "\nQueuing: " << queueCount << "|Total: " << numPersons
						<< std::endl;
				Print() << debugMsgs.str();
				throw std::runtime_error(debugMsgs.str());
			}
		}
	}
}

bool LaneStats::removePerson(Person_MT* p, bool wasQueuing, double vehicleLength)
{
	PersonList::iterator pIt = std::find(laneAgents.begin(), laneAgents.end(), p);
	if (pIt != laneAgents.end())
	{
		laneAgents.erase(pIt);
		if (!laneInfinity)
		{
			numPersons--; //record removal
			totalLength = totalLength - vehicleLength;
			if (wasQueuing)
			{
				if (queueCount > 0)
				{
					queueCount--;
					queueLength = queueLength - vehicleLength;
				}
				else
				{
					std::stringstream debugMsgs;
					debugMsgs << "Error in removePerson(): queueCount cannot be lesser than 0 in lane." << "\nlane:" << lane->getLaneId() << "|Segment: "
							<< lane->getParentSegment()->getRoadSegmentId() << "|Person: " << p->getId() << "\nQueuing: " << queueCount << "|Total: "
							<< laneAgents.size() << std::endl;
					Print() << debugMsgs.str();
					throw std::runtime_error(debugMsgs.str());
				}
			}
		}
		return true;
	}
	return false;
}

void LaneStats::initLaneParams(double vehSpeed, const double capacity)
{
	size_t numLanes = lane->getParentSegment()->getLanes().size();
	if (numLanes > 0)
	{
		double orig = capacity / numLanes;
		laneParams->setOrigOutputFlowRate(orig);
	}
	laneParams->outputFlowRate = laneParams->origOutputFlowRate;

	// As per Yang Lu's suggestion for short segment correction
	if (length < SHORT_SEGMENT_LENGTH_LIMIT)
	{
		laneParams->origOutputFlowRate = LARGE_OUTPUT_FLOW_RATE;
		laneParams->outputFlowRate = LARGE_OUTPUT_FLOW_RATE;
	}

	updateOutputCounter();
	updateAcceptRate(vehSpeed, numLanes);
}

void LaneStats::updateOutputFlowRate(double newFlowRate)
{
	laneParams->outputFlowRate = newFlowRate;
}

void LaneStats::updateOutputCounter()
{
	double tick_size = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	int tmp = int(laneParams->outputFlowRate * tick_size);
	laneParams->fraction += laneParams->outputFlowRate * tick_size - tmp;
	if (laneParams->fraction >= 1.0)
	{
		laneParams->fraction -= 1.0;
		laneParams->outputCounter = tmp + 1;
	}
	else
	{
		laneParams->outputCounter = tmp;
	}
}

void LaneStats::updateAcceptRate(double speed, unsigned int numLanes)
{
	double acceptRateA = (laneParams->outputFlowRate > 0) ? (1.0 / laneParams->outputFlowRate) : 0.0;
	double acceptRateB = PASSENGER_CAR_UNIT / (numLanes * speed);
	laneParams->acceptRate = std::max(acceptRateA, acceptRateB);
}

LaneParams* SegmentStats::getLaneParams(const Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::getLaneParams lane not found in segment stats");
	}
	return laneIt->second->laneParams;
}

double SegmentStats::speedDensityFunction(const double segDensity) const
{
	//maxSpeed according to AIMSUN
	const double freeFlowSpeed = supplyParams.getFreeFlowSpeed();
	const double minSpeed = supplyParams.getMinSpeed();
	const double jamDensity = supplyParams.getJamDensity();
	const double alpha = supplyParams.getAlpha();
	const double beta = supplyParams.getBeta();
	const double minDensity = supplyParams.getMinDensity();

	double speed = 0.0;
	//Speed-Density function same as in DynaMIT
	if (segDensity >= jamDensity)
	{
		speed = minSpeed;
	}
	else if (segDensity >= minDensity)
	{
		speed = freeFlowSpeed * pow((1 - pow((segDensity - minDensity) / jamDensity, beta)), alpha);
	}
	else
	{
		speed = freeFlowSpeed;
	}
	speed = std::max(speed, minSpeed);

	// As per Yang Lu's suggestion for short segment correction
	if (length < SHORT_SEGMENT_LENGTH_LIMIT)
	{
		speed = freeFlowSpeed;
	}
	return speed;
}

void SegmentStats::restoreLaneParams(const Lane* lane)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::restoreLaneParams lane not found in segment stats");
	}
	LaneStats* laneStats = laneIt->second;
	laneStats->updateOutputFlowRate(getLaneParams(lane)->origOutputFlowRate);
	laneStats->updateOutputCounter();
	segDensity = getDensity(true);
	double upSpeed = speedDensityFunction(segDensity);
	laneStats->updateAcceptRate(upSpeed, numVehicleLanes);
}

void SegmentStats::updateLaneParams(const Lane* lane, double newOutputFlowRate)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::updateLaneParams lane not found in segment stats");
	}
	LaneStats* laneStats = laneIt->second;
	laneStats->updateOutputFlowRate(newOutputFlowRate);
	laneStats->updateOutputCounter();
	segDensity = getDensity(true);
	double upSpeed = speedDensityFunction(segDensity);
	laneStats->updateAcceptRate(upSpeed, numVehicleLanes);
}

void SegmentStats::updateLaneParams(timeslice frameNumber)
{
	segDensity = getDensity(true);
	segVehicleSpeed = speedDensityFunction(segDensity);
	//need to update segPedSpeed in future
	LaneStatsMap::iterator it = laneStatsMap.begin();
	for (; it != laneStatsMap.end(); ++it)
	{
		//filtering out the pedestrian lanes for now
		if (!(it->first)->isPedestrianLane())
		{
			(it->second)->updateOutputCounter();
			(it->second)->updateAcceptRate(segVehicleSpeed, numVehicleLanes);
			(it->second)->setInitialQueueLength(it->second->getQueueLength());
		}
	}
}

std::string SegmentStats::reportSegmentStats(uint32_t frameNumber)
{
	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled())
	{
		char segStatBuf[100];
		sprintf(segStatBuf, "seg,%u,%u,%u,%.2f,%u,%.2f,%u,%.2f,%u,%.2f,%u,%.2f,%d,%.2f\n",
				frameNumber,
				roadSegment->getRoadSegmentId(),
				statsNumberInSegment,
				segVehicleSpeed,
				segFlow,
				getTotalDensity(true),
				(numPersons - numAgentsInLane(laneInfinity)),
				getTotalVehicleLength(),
				numMovingInSegment(true),
				getMovingLength(),
				numQueuingInSegment(true),
				getQueueLength(),
				numVehicleLanes,
				length);
		if(getTotalDensity(true)>0)
		{
			int debug =1 ;
		}
		return std::string(segStatBuf);

	}
	else
	{
		return std::string();
	}

//	std::stringstream msg("");
//	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled())
//	{
//		msg << "(\"segmentState\"" << ","
//				<< frameNumber << ","
//				<< roadSegment->getRoadSegmentId()
//				<< ",{" << "\"speed\":\"" << segVehicleSpeed
//				<< "\",\"flow\":\"" << segFlow
//				<< "\",\"density\":\"" << getTotalDensity(true)
//				<< "\",\"total\":\"" << (numPersons - numAgentsInLane(laneInfinity))
//				<< "\",\"totalL\":\"" << getTotalVehicleLength()
//				<< "\",\"moving\":\"" << numMovingInSegment(true)
//				<< "\",\"movingL\":\"" << getMovingLength()
//				<< "\",\"queue\":\"" << numQueuingInSegment(true)
//				<< "\",\"queueL\":\"" << getQueueLength()
//				<< "\",\"numVehicleLanes\":\"" << numVehicleLanes
//				<< "\",\"segment_length\":\"" << length
//				<< "\",\"segment_id\":\"" << roadSegment->getRoadSegmentId()
//				<< "\",\"stats_num\":\"" << statsNumberInSegment << "\"})"
//				<< "\n";
//	}
//	return msg.str();
}

bool SegmentStats::hasPersons() const
{
	return (numPersons > 0);
}

bool SegmentStats::hasBusStop(const BusStop* busStop) const
{
	if (!busStop)
	{
		return false;
	}
	BusStopList::const_iterator stopIt = std::find(busStops.begin(), busStops.end(), busStop);
	return !(stopIt == busStops.end());
}

bool SegmentStats::hasTaxiStand(const TaxiStand *taxiStand) const
{
	if(!taxiStand)
	{
		return false;
	}
	TaxiStandList::const_iterator taxiStandItr = std::find(taxiStands.begin(),taxiStands.end(),taxiStand);
	return (taxiStandItr != taxiStands.end());
}

bool SegmentStats::hasBusStop() const
{
	return !(busStops.empty());
}

double SegmentStats::getPositionOfLastUpdatedAgentInLane(const Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::getPositionOfLastUpdatedAgentInLane lane not found in segment stats");
	}
	return laneIt->second->getPositionOfLastUpdatedAgent();
}

void SegmentStats::setPositionOfLastUpdatedAgentInLane(double positionOfLastUpdatedAgentInLane, const Lane* lane)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::setPositionOfLastUpdatedAgentInLane lane not found in segment stats");
	}
	laneIt->second->setPositionOfLastUpdatedAgent(positionOfLastUpdatedAgentInLane);
}

std::map<const Lane*, LaneStats*> SegmentStats::getLaneStats() const
{
	return laneStatsMap;
}

double SegmentStats::getInitialQueueLength(const Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::getInitialQueueLength lane not found in segment stats");
	}
	return laneIt->second->getInitialQueueLength();
}

void SegmentStats::resetPositionOfLastUpdatedAgentOnLanes()
{
	for (LaneStatsMap::iterator i = laneStatsMap.begin(); i != laneStatsMap.end(); i++)
	{
		i->second->setPositionOfLastUpdatedAgent(-1.0);
	}
}

void SegmentStats::incrementSegFlow()
{
	segFlow++;
}

void SegmentStats::resetSegFlow()
{
	segFlow = 0;
}

unsigned int SegmentStats::computeExpectedOutputPerTick()
{
	float count = 0;
	for (LaneStatsMap::iterator i = laneStatsMap.begin(); i != laneStatsMap.end(); i++)
	{
		count += i->second->laneParams->getOutputFlowRate() * ConfigManager::GetInstance().FullConfig().baseGranSecond();
	}
	return std::floor(count);
}

void SegmentStats::updateLinkDrivingTimes(double drivingTimeToEndOfLink)
{
	double speed = getSegSpeed(true);
	//If speed is 0, treat it as a very small value
	if (speed < INFINITESIMAL_DOUBLE)
	{
		speed = INFINITESIMAL_DOUBLE;
	}

	for (std::vector<Lane*>::const_iterator lnIt = roadSegment->getLanes().begin(); lnIt != roadSegment->getLanes().end(); lnIt++)
	{
		PersonList& lnAgents = laneStatsMap.find(*lnIt)->second->laneAgents;
		for (PersonList::const_iterator pIt = lnAgents.begin(); pIt != lnAgents.end(); pIt++)
		{
			Person_MT* person = (*pIt);
			person->drivingTimeToEndOfLink = (person->distanceToEndOfSegment / speed) + drivingTimeToEndOfLink;
		}
	}
	PersonList& lnAgents = laneStatsMap.find(laneInfinity)->second->laneAgents;
	for (PersonList::const_iterator pIt = lnAgents.begin(); pIt != lnAgents.end(); pIt++)
	{
		Person_MT* person = (*pIt);
		person->drivingTimeToEndOfLink = (person->distanceToEndOfSegment / speed) + drivingTimeToEndOfLink;
	}
}

void SegmentStats::printAgents() const
{
	Print() << "\nSegment: " << roadSegment->getRoadSegmentId() << "|stats#: " << statsNumberInSegment << "|length " << length << std::endl;
	for (LaneStatsMap::const_iterator i = laneStatsMap.begin(); i != laneStatsMap.end(); i++)
	{
		(*i).second->printAgents();
	}

	std::stringstream debugMsgs;
	for (BusStopList::const_reverse_iterator stopIt = busStops.rbegin(); stopIt != busStops.rend(); stopIt++)
	{
		const BusStop* stop = *stopIt;
		debugMsgs << "Stop: " << stop->getStopCode();
		const PersonList& driversAtStop = busDrivers.at(stop);
		for (PersonList::const_iterator pIt = driversAtStop.begin(); pIt != driversAtStop.end(); pIt++)
		{
			debugMsgs << "|" << (*pIt)->getId() << "(" << (*pIt)->busLine << ")";
		}
		debugMsgs << std::endl;
	}
	Print() << debugMsgs.str();
}

void SegmentStats::printBusStops() const
{
	std::stringstream printStream;
	printStream << "Segment: " << roadSegment->getRoadSegmentId() << "|link: " << roadSegment->getParentLink()->getLinkId() << "|stats#: " << statsNumberInSegment
			<< "|length: " << length << "|numStops: " << busStops.size() << "|stops: ";
	if (!busStops.empty())
	{
		for (BusStopList::const_iterator it = busStops.begin(); it != busStops.end(); it++)
		{
			printStream << (*it)->getStopCode() << "\t";
		}
	}
	Print() << printStream.str() << std::endl;
}

void SegmentStats::registerBusStopAgents()
{
	for (BusStopAgentList::iterator stopIt = busStopAgents.begin(); stopIt != busStopAgents.end(); stopIt++)
	{
		messaging::MessageBus::RegisterHandler(*stopIt);
	}
}

bool SegmentStats::isConnectedToDownstreamLink(const Link* downstreamLink, const Lane* lane) const
{
	if (!downstreamLink)
	{
		return false;
	}
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("SegmentStats::getInitialQueueLength lane not found in segment stats");
	}
	const std::set<const Link*>& downStreamLinks = laneIt->second->getDownstreamLinks();
	return (downStreamLinks.find(downstreamLink) != downStreamLinks.end());
}

double SegmentStats::getAllowedVehicleLengthForLaneGroup(const Link* downstreamLink) const
{
	size_t numLanes = getRoadSegment()->getLanes().size();
	if (!downstreamLink)
	{
		return (numLanes * length);
	}
	std::map<const Link*, std::vector<LaneStats*> >::const_iterator lnGpIt = laneGroup.find(downstreamLink);
	if (lnGpIt == laneGroup.end())
	{
		Print() << "DownstreamLink first seg: " << downstreamLink->getLinkId() << std::endl;
		throw std::runtime_error("Invalid downstream link");
	}
	size_t numLanesInLG = lnGpIt->second.size();

	if (numLanes == numLanesInLG)
	{
		return (numLanesInLG * length);
	}
	else if (numLanes < numLanesInLG)
	{
		throw std::runtime_error("numLanes is lesser than numLanesInLaneGroup");
	}
	else
	{
		//total length of lanes in LG + half of length of remaining lanes (suggested by Sebastian on 12-May-2015)
		return ((numLanesInLG * length) + ((numLanes - numLanesInLG) * 0.5 * length));
	}
}

double SegmentStats::getVehicleLengthForLaneGroup(const Link* downstreamLink) const
{
	if (!downstreamLink)
	{
		return getTotalVehicleLength();
	}
	double vehicleLength = 0.0;
	std::map<const Link*, std::vector<LaneStats*> >::const_iterator lnGpIt = laneGroup.find(downstreamLink);
	if (lnGpIt == laneGroup.end())
	{
		throw std::runtime_error("Invalid downstream link");
	}
	const std::vector<LaneStats*>& laneStatsInLG = lnGpIt->second;
	for (std::vector<LaneStats*>::const_iterator lnStatsIt = laneStatsInLG.begin(); lnStatsIt != laneStatsInLG.end(); lnStatsIt++)
	{
		vehicleLength = vehicleLength + (*lnStatsIt)->getTotalVehicleLength();
	}
	return vehicleLength;
}

void SegmentStats::printDownstreamLinks() const
{
	std::stringstream out;
	out << "DownStreamLinks of " << roadSegment->getRoadSegmentId() << "-" << statsNumberInSegment << std::endl;
	for (LaneStatsMap::const_iterator i = laneStatsMap.begin(); i != laneStatsMap.end(); i++)
	{
		if (i->second->isLaneInfinity())
		{
			continue;
		}
		out << i->first->getLaneId() << " - ";
		const std::set<const Link*>& downStreamLinks = i->second->getDownstreamLinks();
		for (std::set<const Link*>::const_iterator j = downStreamLinks.begin(); j != downStreamLinks.end(); j++)
		{
			out << (*j)->getLinkId() << "|";
		}
		out << std::endl;
	}
	Print() << out.str();
}

void LaneStats::printAgents() const
{
	std::stringstream debugMsgs;
	debugMsgs << "Lane: " << lane->getLaneId();
	for (PersonList::const_iterator i = laneAgents.begin(); i != laneAgents.end(); i++)
	{
		debugMsgs << "|" << (*i)->getDatabaseId() ;
		if ( (*i)->isQueuing)
		{
			debugMsgs<< "(" << "queuing" << ")";
		}
		if ( (*i)->getRole())
		{
			MovementFacet *movFacet = (*i)->getRole()->Movement();
			TaxiDriverMovement *mov = dynamic_cast<TaxiDriverMovement*>(movFacet);
			if(mov)
			{
				const MesoPathMover pathMover = mov->getMesoPathMover();
				const std::vector<const SegmentStats*>& path = pathMover.getPath();
				debugMsgs << "(pathStats:";
				for(auto i = path.begin(); i!=path.end(); i++)
				{
					debugMsgs << (*i)->getRoadSegment()->getRoadSegmentId()<<"|";
				}
				debugMsgs << ")(currStats:"<<pathMover.getCurrSegStats()->getRoadSegment()->getRoadSegmentId()<<")";
				debugMsgs<< "(" << "posSeg:" << pathMover.getPositionInSegment() << " )" ;
			}
		}
		if((*i)->getPrevRole()){
			debugMsgs << "(" << (*i)->getPrevRole()->getRoleName() << ")";
		}
		if((*i)->getRole()){
			debugMsgs << "(" << (*i)->getRole()->getRoleName() << ")";
		}
		if((*i)->getNextRole()){
			debugMsgs << "(" << (*i)->getNextRole()->getRoleName() << ")";
		}

	}
	debugMsgs << std::endl;
	Print() << debugMsgs.str();
}

void LaneStats::verifyOrdering() const
{
	double distance = -1.0;
	for (PersonList::const_iterator i = laneAgents.begin(); i != laneAgents.end(); i++)
	{
		const Person_MT* person = (*i);
		if (distance > person->distanceToEndOfSegment)
		{
			std::stringstream debugMsgs;
			debugMsgs << "Invariant violated: Ordering of laneAgents does not reflect ordering w.r.t. distance to end of segment."
					<< "\nSegment: " << lane->getParentSegment()->getRoadSegmentId() << "-" << parentStats->getStatsNumberInSegment()
					<< " length = " << lane->getParentSegment()->getLength() << "\nLane: " << lane->getLaneId()
					<< "\nCulprit Person: " << person->getDatabaseId();
			printf("%s", debugMsgs.str().c_str());
			debugMsgs << "\nAgents ";
			for (PersonList::const_iterator j = laneAgents.begin(); j != laneAgents.end(); j++)
			{
				debugMsgs << "|" << (*j)->getDatabaseId() << "(" << (*j)->getRole()->getRoleName() << ")" << "--" << (*j)->distanceToEndOfSegment;
			}
			printf("%s", debugMsgs.str().c_str());
			throw std::runtime_error(debugMsgs.str());
		}
		else
		{
			distance = (*i)->distanceToEndOfSegment;
		}
	}
}

Person_MT* SegmentStats::dequeue(const Person_MT* person, const Lane* lane, bool isQueuingBfrUpdate, double vehicleLength)
{
	if (!person)
	{
		return nullptr;
	}
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if (laneIt == laneStatsMap.end())
	{
		return nullptr;
	}
	Person_MT* dequeuedPerson = laneIt->second->dequeue(person, isQueuingBfrUpdate, vehicleLength);
	if (dequeuedPerson)
	{
		numPersons--; // record removal from segment
	}
	else
	{
		printAgents();
		std::stringstream debugMsgs;
		debugMsgs << "Error: Person " << person->getId() << "|" << person->getDatabaseId() << " (" << person->getRole()->getRoleName() << ")"
				<< " was not found in lane " << lane->getLaneId() << std::endl;
		throw std::runtime_error(debugMsgs.str());
	}
	return dequeuedPerson;
}

Person_MT* LaneStats::dequeue(const Person_MT* person, bool isQueuingBfrUpdate, double vehicleLength)
{
	if (laneAgents.size() == 0)
	{
		std::stringstream debugMsgs;
		debugMsgs << "Trying to dequeue Person " << person->getId() << " from empty lane." << std::endl;
		return nullptr;
	}
	Person_MT* dequeuedPerson = nullptr;
	if (laneInfinity)
	{
		PersonList::iterator it;
		for (it = laneAgents.begin(); it != laneAgents.end(); it++)
		{
			if ((*it) == person)
			{
				dequeuedPerson = (*it);
				it = laneAgents.erase(it); // erase returns the next iterator
				numPersons--; //record removal
				break; //exit loop
			}
		}
	}
	else if (person == laneAgents.front())
	{
		dequeuedPerson = laneAgents.front();
		laneAgents.pop_front();
		numPersons--; // record removal
		totalLength = totalLength - vehicleLength;
		if (isQueuingBfrUpdate)
		{
			if (queueCount > 0)
			{
				// we have removed a queuing agent
				queueCount--;
				queueLength = queueLength - vehicleLength;
			}
			else
			{
				std::stringstream debugMsgs;
				debugMsgs << "Error in dequeue(): queueCount cannot be lesser than 0 in lane." << "\nlane:" << lane->getLaneId() << "|Segment: "
						<< lane->getParentSegment()->getRoadSegmentId() << "|Person: " << dequeuedPerson->getId() << "\nQueuing: " << queueCount << "|Total: "
						<< laneAgents.size() << std::endl;
				Print() << debugMsgs.str();
				throw std::runtime_error(debugMsgs.str());
			}
		}
	}
	if(dequeuedPerson == nullptr)
	{
		PersonList::iterator it;
		for (it = laneAgents.begin(); it != laneAgents.end(); it++)
		{
			Person *per = (*it);
			int debug = 1 ;
		}
	}
	return dequeuedPerson;
}

bool LaneStats::addDownstreamLink(const Link* downStreamLink)
{
	if (downStreamLink)
	{
		return connectedDownstreamLinks.insert(downStreamLink).second;
	}
	return false;
}

void LaneStats::addDownstreamLinks(const std::set<const Link*>& downStreamLinks)
{
	connectedDownstreamLinks.insert(downStreamLinks.begin(), downStreamLinks.end());
}

void LaneParams::decrementOutputCounter()
{
	if (outputCounter > 0)
	{
		outputCounter--;
	}
	else
	{
		throw std::runtime_error("cannot allow vehicles beyond output capacity.");
	}
}

double SegmentStats::getCapacity() const
{
	return supplyParams.getCapacity();
}

unsigned int SegmentStats::getBusWaitersCount() const
{
	unsigned int totalWaiting = 0;
	for(const sim_mob::medium::BusStopAgent* stopAg : busStopAgents)
	{
		totalWaiting += stopAg->getWaitingCount();
	}
	return totalWaiting;
}

bool SegmentStats::isShortSegment() const
{
	return (length < SHORT_SEGMENT_LENGTH_LIMIT);
}
