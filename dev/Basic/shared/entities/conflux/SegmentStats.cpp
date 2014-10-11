//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "SegmentStats.hpp"

#include <cmath>
#include <algorithm>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/roles/Role.hpp"
#include "entities/vehicle/VehicleBase.hpp"
#include "geospatial/Link.hpp"
#include "logging/Log.hpp"
#include "message/MessageBus.hpp"

using std::string;

namespace
{
const double INFINITESIMAL_DOUBLE = 0.000001;
const double SHORT_SEGMENT_LENGTH_LIMIT = 5 * sim_mob::PASSENGER_CAR_UNIT; // 5 times a car's length
const double LARGE_OUTPUT_FLOW_RATE = 1000.0;

const double SINGLE_LANE_SEGMENT_CAPACITY = 1200.0; //veh/hr. suggested by Yang Lu on 11-Oct-2014
const double DOUBLE_LANE_SEGMENT_CAPACITY = 3000.0; //veh/hr. suggested by Yang Lu on 11-Oct-2014

/**
 * converts the unit of speed from Km/h to cm/s
 * @param speedInKmph spped in Km/h
 * @return speed in cm/s
 */
inline double convertKmphToCmps(double speedInKmph)
{
	return (speedInKmph / 3.6 * 100);
}
}

namespace sim_mob
{

bool cmp_person_distToSegmentEnd::operator ()(const Person* x, const Person* y) const
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
//SupplyParams::SupplyParams(const sim_mob::RoadSegment* rdSeg, double statsLength) :
//		freeFlowSpeed(convertKmphToCmps(rdSeg->maxSpeed)),
//		minSpeed(0.3 * freeFlowSpeed), /*30% of free flow speed as suggested by Yang Lu*/
//		jamDensity(0.2), /*density during traffic jam in veh/meter*/
//		minDensity(0.0048), /*minimum traffic density in veh/meter*/
//		capacity(rdSeg->getCapacity() / 3600.0), /*converting capacity to vehicles/hr to vehicles/s*/
//		alpha(1.8), beta(1.9)
//{}

/*
 * The parameter values for min density, jam density, alpha and beta were suggested by Yang Lu on 11-Oct-14
 */
SupplyParams::SupplyParams(const sim_mob::RoadSegment* rdSeg, double statsLength) :
		freeFlowSpeed(convertKmphToCmps(rdSeg->maxSpeed)),
		minSpeed(0.2 * freeFlowSpeed), /*20% of free flow speed as suggested by Yang Lu*/
		jamDensity(0.25), /*density during traffic jam in veh/meter*/
		minDensity(0.0048), /*minimum traffic density in veh/meter*/
		capacity(rdSeg->getCapacity() / 3600.0), /*converting capacity to vehicles/hr to vehicles/s*/
		alpha(1.0), beta(2.5)
{
	//update capacity of single and double lane segments to avoid bottle necks. Suggested by Yang Lu on 11-Oct-14
	if(rdSeg->getLanes().size() == 1) { capacity = SINGLE_LANE_SEGMENT_CAPACITY/3600.0; }
	else if(rdSeg->getLanes().size() == 2) { capacity = DOUBLE_LANE_SEGMENT_CAPACITY/3600.0; }
}

SegmentStats::SegmentStats(const sim_mob::RoadSegment* rdSeg, double statslength) :
		roadSegment(rdSeg), length(statslength), segDensity(0.0), segPedSpeed(0.0), segFlow(0), numPersons(0), statsNumberInSegment(1), supplyParams(rdSeg,
				statslength), orderBySetting(SEGMENT_ORDERING_BY_DISTANCE_TO_INTERSECTION), debugMsgs(std::stringstream::out)
{
	segVehicleSpeed = convertKmphToCmps(getRoadSegment()->maxSpeed);
	numVehicleLanes = 0;

	// initialize LaneAgents in the map
	std::vector<sim_mob::Lane*>::const_iterator laneIt = rdSeg->getLanes().begin();
	while (laneIt != rdSeg->getLanes().end())
	{
		laneStatsMap.insert(std::make_pair(*laneIt, new sim_mob::LaneStats(*laneIt, length)));
		laneStatsMap[*laneIt]->initLaneParams(segVehicleSpeed, supplyParams.getCapacity());
		if (!(*laneIt)->is_pedestrian_lane())
		{
			numVehicleLanes++;
			outermostLane = *laneIt;
		}
		laneIt++;
	}

	/*
	 * Any lane with an id ending with 9 is laneInfinity of the road segment.
	 * This lane is available only to the SegmentStats and not the parent RoadSegment.
	 * TODO: Must check if we can have a bit pattern (Refer lane constructor) for laneInfinity.
	 */
	laneInfinity = new sim_mob::Lane(const_cast<sim_mob::RoadSegment*>(rdSeg), 9);
	laneStatsMap.insert(std::make_pair(laneInfinity, new sim_mob::LaneStats(laneInfinity, statslength, true)));
}

SegmentStats::~SegmentStats()
{
	for (LaneStatsMap::iterator i = laneStatsMap.begin(); i != laneStatsMap.end(); i++)
	{
		safe_delete_item(i->second);
	}
	for (AgentList::iterator i = busStopAgents.begin(); i != busStopAgents.end(); i++)
	{
		safe_delete_item(*i);
	}
	safe_delete_item(laneInfinity);
}

void SegmentStats::updateBusStopAgents(timeslice now)
{
	for (AgentList::iterator i = busStopAgents.begin(); i != busStopAgents.end(); i++)
	{
		(*i)->update(now);
	}
}

void SegmentStats::addAgent(const sim_mob::Lane* lane, sim_mob::Person* p) {
	laneStatsMap.find(lane)->second->addPerson(p);
	numPersons++; //record addition to segment
}

void SegmentStats::removeAgent(const sim_mob::Lane* lane, sim_mob::Person* p, bool wasQueuing)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	laneIt->second->removePerson(p, wasQueuing);
	numPersons--; //record removal from segment
}

void SegmentStats::updateQueueStatus(const sim_mob::Lane* lane, sim_mob::Person* p)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	laneIt->second->updateQueueStatus(p);
}

std::deque<sim_mob::Person*>& SegmentStats::getPersons(const sim_mob::Lane* lane)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	return laneIt->second->laneAgents;
}

std::vector<const sim_mob::BusStop*>& SegmentStats::getBusStops()
{
	return busStops;
}

void SegmentStats::addBusStopAgent(sim_mob::Agent* busStopAgent)
{
	if(!busStopAgent) { return; }
	busStopAgents.push_back(busStopAgent);
}

void SegmentStats::initializeBusStops()
{
	for (AgentList::iterator stopAgIt=busStopAgents.begin(); stopAgIt!=busStopAgents.end(); stopAgIt++)
	{
		Agent* stopAgent = *stopAgIt;
		if (!stopAgent->isInitialized())
		{
			messaging::MessageBus::RegisterHandler(stopAgent);
			stopAgent->setInitialized(true);
		}
	}
}

void SegmentStats::addBusDriverToStop(sim_mob::Person* driver, const sim_mob::BusStop* stop)
{
	if (stop && hasBusStop(stop))
	{
		busDrivers.at(stop).push_back(driver);
	}
}

void SegmentStats::removeBusDriverFromStop(sim_mob::Person* driver, const sim_mob::BusStop* stop)
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

std::deque<sim_mob::Person*> SegmentStats::getPersons()
{
	PersonList segAgents;
	for (std::vector<sim_mob::Lane*>::const_iterator lnIt = roadSegment->getLanes().begin(); lnIt != roadSegment->getLanes().end(); lnIt++)
	{
		PersonList& lnAgents = laneStatsMap.find(*lnIt)->second->laneAgents;
		segAgents.insert(segAgents.end(), lnAgents.begin(), lnAgents.end());
	}
	PersonList& lnAgents = laneStatsMap.find(laneInfinity)->second->laneAgents;
	segAgents.insert(segAgents.end(), lnAgents.begin(), lnAgents.end());

	for (BusStopList::const_reverse_iterator stopIt = busStops.rbegin(); stopIt != busStops.rend(); stopIt++)
	{
		const sim_mob::BusStop* stop = *stopIt;
		PersonList& driversAtStop = busDrivers.at(stop);
		segAgents.insert(segAgents.end(), driversAtStop.begin(), driversAtStop.end());
	}
	return segAgents;
}

void SegmentStats::topCMergeLanesInSegment(PersonList& mergedPersonList)
{
	int capacity = (int) (ceil(roadSegment->getCapacityPerInterval()));
	std::vector<PersonList::iterator> iteratorLists;

	//init iterator list to the front of each lane
	for (LaneStatsMap::iterator lnIt = laneStatsMap.begin(); lnIt != laneStatsMap.end(); lnIt++)
	{
		iteratorLists.push_back(lnIt->second->laneAgents.begin());
	}

	//pick the Top C
	for (int c = 0; c < capacity; c++)
	{
		int dequeIndex = -1;
		double minDistance = std::numeric_limits<double>::max();
		sim_mob::Person* minPerson = nullptr;
		int i = 0;
		for (LaneStatsMap::iterator lnIt = laneStatsMap.begin(); lnIt != laneStatsMap.end(); lnIt++)
		{
			PersonList& personsInLane = lnIt->second->laneAgents;
			//order by location
			if (orderBySetting == SEGMENT_ORDERING_BY_DISTANCE_TO_INTERSECTION)
			{
				if (iteratorLists[i] != personsInLane.end() && (*iteratorLists[i])->distanceToEndOfSegment < minDistance)
				{
					dequeIndex = i;
					minPerson = (*(iteratorLists[i]));
					minDistance = minPerson->distanceToEndOfSegment;
				}
			}
			//order by time
			else if (orderBySetting == SEGMENT_ORDERING_BY_DRIVING_TIME_TO_INTERSECTION)
			{
				if (iteratorLists[i] != personsInLane.end() && (*iteratorLists[i])->drivingTimeToEndOfLink < minDistance)
				{
					dequeIndex = i;
					minPerson = (*(iteratorLists[i]));
					minDistance = minPerson->drivingTimeToEndOfLink;
				}
			}
			i++;
		}

		if (dequeIndex < 0)
		{
			return; //no more vehicles
		}
		else
		{
			iteratorLists.at(dequeIndex)++;
			mergedPersonList.push_back(minPerson);
		}
	}

	//After picking the Top C, just append the remaining vehicles in the output list
	int i = 0;
	for (LaneStatsMap::iterator lnIt = laneStatsMap.begin(); lnIt != laneStatsMap.end(); lnIt++)
	{
		PersonList& personsInLane = lnIt->second->laneAgents;
		if (iteratorLists[i] != personsInLane.end())
		{
			mergedPersonList.insert(mergedPersonList.end(), iteratorLists[i], personsInLane.end());
		}
		i++;
	}

	//And let's not forget the bus drivers serving stops in this segment stats
	//Bus drivers go in the front of the list, because bus stops are (virtually)
	//located at the end of the segment
	for (BusStopList::const_reverse_iterator stopIt = busStops.rbegin(); stopIt != busStops.rend(); stopIt++)
	{
		const sim_mob::BusStop* stop = *stopIt;
		PersonList& driversAtStop = busDrivers.at(stop);
		for (PersonList::iterator pIt = driversAtStop.begin(); pIt != driversAtStop.end(); pIt++)
		{
			mergedPersonList.push_front(*pIt);
		}
	}
}

std::pair<unsigned int, unsigned int> SegmentStats::getLaneAgentCounts(const sim_mob::Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	return std::make_pair(laneIt->second->getQueuingAgentsCount(), laneIt->second->getMovingAgentsCount());
}

double SegmentStats::getLaneQueueLength(const sim_mob::Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	return laneIt->second->getQueueLength();
}

double SegmentStats::getLaneMovingLength(const sim_mob::Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	return laneIt->second->getMovingLength();
}

double SegmentStats::getLaneTotalVehicleLength(const sim_mob::Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	return laneIt->second->getTotalVehicleLength();
}

unsigned int SegmentStats::numAgentsInLane(const sim_mob::Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	return laneIt->second->getNumPersons();
}

unsigned int SegmentStats::numMovingInSegment(bool hasVehicle) const
{
	unsigned int movingCounts = 0;
	const std::vector<sim_mob::Lane*>& segLanes = roadSegment->getLanes();
	std::vector<sim_mob::Lane*>::const_iterator laneIt = segLanes.begin();
	while (laneIt != segLanes.end())
	{
		if ((hasVehicle && !(*laneIt)->is_pedestrian_lane()) || (!hasVehicle && (*laneIt)->is_pedestrian_lane()))
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
		if (!laneStatsIt->second->isLaneInfinity() && !laneStatsIt->first->is_pedestrian_lane())
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
		if (!laneStatsIt->second->isLaneInfinity() && !laneStatsIt->first->is_pedestrian_lane())
		{
			queueLength = queueLength + laneStatsIt->second->getQueueLength();
		}
	}
	return queueLength;
}

double SegmentStats::getTotalVehicleLength() const
{
	double totalLength = 0;
	for (LaneStatsMap::const_iterator laneStatsIt = laneStatsMap.begin(); laneStatsIt != laneStatsMap.end(); laneStatsIt++)
	{
		if (!laneStatsIt->second->isLaneInfinity() && !laneStatsIt->first->is_pedestrian_lane())
		{
			totalLength = totalLength + laneStatsIt->second->getTotalVehicleLength();
		}
	}
	return totalLength;
}

//density will be computed in vehicles/meter for the moving part of the segment
double SegmentStats::getDensity(bool hasVehicle)
{
	double density = 0.0;
	double movingPartLength = length * numVehicleLanes - getQueueLength();
	double movingPCUs = getMovingLength() / PASSENGER_CAR_UNIT;
	if (movingPartLength > PASSENGER_CAR_UNIT)
	{
		/*Some lines in this if section are commented as per Yang Lu's suggestion */
		//if (roadSegment->getLaneZeroLength() > 10*vehicle_length) {
		density = movingPCUs / (movingPartLength / 100.0);
		//}
		//else {
		//	density = queueCount/(movingLength/100.0);
		//}
	}
	else
	{
		density = 1 / (PASSENGER_CAR_UNIT / 100.0);
	}
	return density;
}

unsigned int SegmentStats::numQueuingInSegment(bool hasVehicle) const
{
	unsigned int queuingCounts = 0;
	const std::vector<sim_mob::Lane*>& segLanes = roadSegment->getLanes();
	std::vector<sim_mob::Lane*>::const_iterator lane = segLanes.begin();
	while (lane != segLanes.end())
	{
		if ((hasVehicle && !(*lane)->is_pedestrian_lane()) || (!hasVehicle && (*lane)->is_pedestrian_lane()))
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

sim_mob::Person* SegmentStats::personClosestToSegmentEnd()
{
	sim_mob::Person* person = nullptr;
	const sim_mob::Lane* personLane = nullptr;
	double minDistance = std::numeric_limits<double>::max();

	std::map<const sim_mob::Lane*, sim_mob::Person*>::iterator i = frontalAgents.begin();
	while (i != frontalAgents.end())
	{
		if (i->second)
		{
			if (minDistance == i->second->distanceToEndOfSegment)
			{
				// If current person and (*i) are at equal distance to the stop line, we 'toss a coin' and choose one of them
				bool coinTossResult = ((rand() / (double) RAND_MAX) < 0.5);
				if (coinTossResult)
				{
					personLane = i->first;
					person = i->second;
				}
			}
			else if (minDistance > i->second->distanceToEndOfSegment)
			{
				minDistance = i->second->distanceToEndOfSegment;
				personLane = i->first;
				person = i->second;
			}
		}
		i++;
	}

	if (person)
	{ // frontalAgents could possibly be all nullptrs
		frontalAgents.erase(personLane);
		frontalAgents.insert(std::make_pair(personLane, laneStatsMap.at(personLane)->next()));
	}
	return person;
}

void SegmentStats::resetFrontalAgents()
{
	frontalAgents.clear();
	for (LaneStatsMap::iterator i = laneStatsMap.begin(); i != laneStatsMap.end(); i++)
	{
		i->second->resetIterator();
		Person* person = i->second->next();
		frontalAgents.insert(std::make_pair(i->first, person));
	}
}

void SegmentStats::addBusStop(const sim_mob::BusStop* stop)
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

sim_mob::Person* LaneStats::next()
{
	sim_mob::Person* person = nullptr;
	if (laneAgentsIt != laneAgentsCopy.end())
	{
		person = *laneAgentsIt;
		laneAgentsIt++;
	}
	return person;
}

unsigned int sim_mob::LaneStats::getQueuingAgentsCount() const
{
	return queueCount;
}

unsigned int sim_mob::LaneStats::getMovingAgentsCount() const
{
	if (numPersons < queueCount)
	{
		printAgents();
		std::stringstream debugMsgs;
		debugMsgs << "number of lane agents cannot be less than the number of queuing agents." << "\nlane" << getLane()->getLaneID() << "|queueCount: "
				<< queueCount << "|laneAgents count: " << numPersons << std::endl;
		throw std::runtime_error(debugMsgs.str());
	}
	return (numPersons - queueCount);
}

double sim_mob::LaneStats::getTotalVehicleLength() const
{
	return totalLength;
}

double sim_mob::LaneStats::getQueueLength() const
{
	return queueLength;
}

double sim_mob::LaneStats::getMovingLength() const
{
	if (totalLength < queueLength)
	{
		printAgents();
		std::stringstream debugMsgs;
		debugMsgs << "totalLength cannot be less than queueLength." << "\nlane" << getLane()->getLaneID() << "|queueLength: " << queueLength << "|totalLength: "
				<< totalLength << std::endl;
		throw std::runtime_error(debugMsgs.str());
	}
	return (totalLength - queueLength);
}

void sim_mob::LaneStats::addPerson(sim_mob::Person* p)
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
			std::deque<Person*>::iterator i = laneAgents.end() - 1; // last person's iterator
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
			totalLength = totalLength + vehicle->getLengthCm();
			if (p->isQueuing)
			{
				queueCount++;
				queueLength = queueLength + vehicle->getLengthCm();
			}
		}
	}
}

void sim_mob::LaneStats::updateQueueStatus(sim_mob::Person* p)
{
	VehicleBase* vehicle = p->getRole()->getResource();
	if (!laneInfinity && vehicle)
	{
		if (p->isQueuing)
		{
			queueCount++;
			queueLength = queueLength + vehicle->getLengthCm();
		}
		else
		{
			if (queueCount > 0)
			{
				queueCount--;
				queueLength = queueLength - vehicle->getLengthCm();
			}
			else
			{
				std::stringstream debugMsgs;
				debugMsgs << "Error in updateQueueStatus(): queueCount cannot be lesser than 0 in lane." << "\nlane:" << lane->getLaneID() << "|Segment: "
						<< lane->getRoadSegment()->getStartEnd() << "|Person: " << p->getId() << "\nQueuing: " << queueCount << "|Total: " << numPersons
						<< std::endl;
				Print() << debugMsgs.str();
				throw std::runtime_error(debugMsgs.str());
			}
		}
	}
}

void sim_mob::LaneStats::removePerson(sim_mob::Person* p, bool wasQueuing)
{
	PersonList::iterator pIt = std::find(laneAgents.begin(), laneAgents.end(), p);
	VehicleBase* vehicle = p->getRole()->getResource();
	if (pIt != laneAgents.end())
	{
		laneAgents.erase(pIt);
		if (!laneInfinity && vehicle)
		{
			numPersons--; //record removal
			totalLength = totalLength - vehicle->getLengthCm();
			if (wasQueuing)
			{
				if (queueCount > 0)
				{
					queueCount--;
					queueLength = queueLength - vehicle->getLengthCm();
				}
				else
				{
					std::stringstream debugMsgs;
					debugMsgs << "Error in removePerson(): queueCount cannot be lesser than 0 in lane." << "\nlane:" << lane->getLaneID() << "|Segment: "
							<< lane->getRoadSegment()->getStartEnd() << "|Person: " << p->getId() << "\nQueuing: " << queueCount << "|Total: " << laneAgents.size()
							<< std::endl;
					Print() << debugMsgs.str();
					throw std::runtime_error(debugMsgs.str());
				}
			}
		}
	}
	else
	{
		throw std::runtime_error("LaneStats::removePerson(): Attempt to remove non-existent person in Lane");
	}
}

void LaneStats::resetIterator()
{
	laneAgentsCopy = laneAgents;
	laneAgentsIt = laneAgentsCopy.begin();
}

void sim_mob::LaneStats::initLaneParams(double vehSpeed, const double capacity)
{
	size_t numLanes = lane->getRoadSegment()->getLanes().size();
	if (numLanes > 0)
	{
		double orig = capacity / numLanes;
		laneParams->setOrigOutputFlowRate(orig);
	}
	laneParams->outputFlowRate = laneParams->origOutputFlowRate;

	// As per Yang Lu's suggestion for short segment correction
	if (length < SHORT_SEGMENT_LENGTH_LIMIT)
	{
		laneParams->outputFlowRate = LARGE_OUTPUT_FLOW_RATE; //some large number
	}

	updateOutputCounter();
	updateAcceptRate(vehSpeed);
}

void sim_mob::LaneStats::updateOutputFlowRate(double newFlowRate)
{
	laneParams->outputFlowRate = newFlowRate;
}

void sim_mob::LaneStats::updateOutputCounter()
{
	double tick_size = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	int tmp = int(laneParams->outputFlowRate * tick_size);
	laneParams->fraction += laneParams->outputFlowRate * tick_size - float(tmp);
	if (laneParams->fraction >= 1.0)
	{
		laneParams->fraction -= 1.0;
		laneParams->outputCounter = float(tmp) + 1.0;
	}
	else
	{
		laneParams->outputCounter = float(tmp);
	}
}

void sim_mob::LaneStats::updateAcceptRate(double upSpeed)
{
	const double omega = 0.01;
	double tick_size = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	double capacity = laneParams->outputFlowRate * tick_size;
	double acceptRateA = (capacity > 0) ? tick_size / capacity : 0;
	double acceptRateB = (omega * PASSENGER_CAR_UNIT) / upSpeed;
	laneParams->acceptRate = std::max(acceptRateA, acceptRateB);
}

sim_mob::LaneParams* sim_mob::SegmentStats::getLaneParams(const Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	return laneIt->second->laneParams;
}

double sim_mob::SegmentStats::speedDensityFunction(const double segDensity) const
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

void sim_mob::SegmentStats::restoreLaneParams(const Lane* lane)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	LaneStats* laneStats = laneIt->second;
	laneStats->updateOutputFlowRate(getLaneParams(lane)->origOutputFlowRate);
	laneStats->updateOutputCounter();
	segDensity = getDensity(true);
	double upSpeed = speedDensityFunction(segDensity);
	laneStats->updateAcceptRate(upSpeed);
}

void sim_mob::SegmentStats::updateLaneParams(const Lane* lane, double newOutputFlowRate)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	LaneStats* laneStats = laneIt->second;
	laneStats->updateOutputFlowRate(newOutputFlowRate);
	laneStats->updateOutputCounter();
	segDensity = getDensity(true);
	double upSpeed = speedDensityFunction(segDensity);
	laneStats->updateAcceptRate(upSpeed);
}

void sim_mob::SegmentStats::updateLaneParams(timeslice frameNumber)
{
	segDensity = getDensity(true);
	segVehicleSpeed = speedDensityFunction(segDensity);
	//need to update segPedSpeed in future
	LaneStatsMap::iterator it = laneStatsMap.begin();
	for (; it != laneStatsMap.end(); ++it)
	{
		//filtering out the pedestrian lanes for now
		if (!(it->first)->is_pedestrian_lane())
		{
			(it->second)->updateOutputCounter();
			(it->second)->updateAcceptRate(segVehicleSpeed);
			(it->second)->setInitialQueueLength(it->second->getQueueLength());
		}
	}
}

std::string sim_mob::SegmentStats::reportSegmentStats(timeslice frameNumber)
{
	std::stringstream msg;
	double density = segDensity	* 1000.0; /* Density is converted to veh/km/seg for the output */

#define SHOW_NUMBER_VEHICLE_ON_SEGMENT
#ifdef SHOW_NUMBER_VEHICLE_ON_SEGMENT

	msg << "(\"segmentState\""
		<< "," << frameNumber.frame()
		<< "," << roadSegment
		<< ",{"
		<< "\"speed\":\"" << segVehicleSpeed
		<< "\",\"flow\":\"" << segFlow
		<< "\",\"density\":\"" << density
		<< "\",\"total\":\"" << (numPersons - numAgentsInLane(laneInfinity))
		<< "\",\"totalL\":\"" << getTotalVehicleLength()
		<< "\",\"moving\":\"" << numMovingInSegment(true)
		<< "\",\"movingL\":\"" << getMovingLength()
		<< "\",\"queue\":\"" << numQueuingInSegment(true)
		<< "\",\"queueL\":\"" << getQueueLength()
		<< "\",\"numVehicleLanes\":\"" << numVehicleLanes
		<< "\",\"segment_length\":\"" << length
		<< "\"})"
		<< std::endl;
#else

	msg <<"(\"segmentState\""
		<<","<<frameNumber.frame()
		<<","<<roadSegment
		<<",{"
		<<"\"speed\":\""<< segVehicleSpeed
		<<"\",\"flow\":\""<< segFlow
		<<"\",\"density\":\""<< density
		<<"\"})"
		<<std::endl;
#endif
	return msg.str();
}

double sim_mob::SegmentStats::getSegSpeed(bool hasVehicle) const
{
	if (hasVehicle)
	{
		return segVehicleSpeed;
	}
	return segPedSpeed;
}

bool SegmentStats::hasPersons() const
{
	return (numPersons > 0);
}

bool SegmentStats::hasBusStop(const sim_mob::BusStop* busStop) const
{
	if(!busStop) { return false; }
	BusStopList::const_iterator stopIt = std::find(busStops.begin(), busStops.end(), busStop);
	return !(stopIt == busStops.end());
}

double SegmentStats::getPositionOfLastUpdatedAgentInLane(const Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	return laneIt->second->getPositionOfLastUpdatedAgent();
}

void SegmentStats::setPositionOfLastUpdatedAgentInLane(double positionOfLastUpdatedAgentInLane, const Lane* lane)
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	laneIt->second->setPositionOfLastUpdatedAgent(positionOfLastUpdatedAgentInLane);
}

unsigned int sim_mob::SegmentStats::getInitialQueueLength(const Lane* lane) const
{
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt==laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
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

unsigned int SegmentStats::getSegFlow()
{
	return segFlow;
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

	for (std::vector<sim_mob::Lane*>::const_iterator lnIt = roadSegment->getLanes().begin(); lnIt != roadSegment->getLanes().end(); lnIt++)
	{
		PersonList& lnAgents = laneStatsMap.find(*lnIt)->second->laneAgents;
		for (PersonList::const_iterator pIt = lnAgents.begin(); pIt != lnAgents.end(); pIt++)
		{
			Person* person = (*pIt);
			person->drivingTimeToEndOfLink = (person->distanceToEndOfSegment / speed) + drivingTimeToEndOfLink;
		}
	}
	PersonList& lnAgents = laneStatsMap.find(laneInfinity)->second->laneAgents;
	for (PersonList::const_iterator pIt = lnAgents.begin(); pIt != lnAgents.end(); pIt++)
	{
		Person* person = (*pIt);
		person->drivingTimeToEndOfLink = (person->distanceToEndOfSegment / speed) + drivingTimeToEndOfLink;
	}
}

void SegmentStats::printAgents() const
{
	Print() << "\nSegment: " << roadSegment->getSegmentAimsunId() << "|stats#: " << statsNumberInSegment << "|length " << length << std::endl;
	for (LaneStatsMap::const_iterator i = laneStatsMap.begin(); i != laneStatsMap.end(); i++)
	{
		(*i).second->printAgents();
	}
	for (LaneStatsMap::const_iterator i = laneStatsMap.begin(); i != laneStatsMap.end(); i++)
	{
		(*i).second->printAgents(true);
	}
	std::stringstream debugMsgs;
	for (BusStopList::const_reverse_iterator stopIt = busStops.rbegin(); stopIt != busStops.rend(); stopIt++)
	{
		const sim_mob::BusStop* stop = *stopIt;
		debugMsgs << "Stop: " << stop->getBusstopno_();
		const PersonList& driversAtStop = busDrivers.at(stop);
		for (PersonList::const_iterator pIt = driversAtStop.begin(); pIt != driversAtStop.end(); pIt++)
		{
			debugMsgs << "|" << (*pIt)->getId();
		}
		debugMsgs << std::endl;
	}
	Print() << debugMsgs.str();
}

void SegmentStats::printBusStops() const
{
	std::stringstream printStream;
	printStream << "Segment: " << roadSegment->getSegmentAimsunId()
			<< "|link: " << roadSegment->getLink()->getLinkId()
			<< "|stats#: " << statsNumberInSegment
			<< "|length: " << length
			<< "|numStops: " << busStops.size()
			<< "|stops: ";
	if (!busStops.empty())
	{
		for (BusStopList::const_iterator it = busStops.begin(); it != busStops.end(); it++)
		{
			printStream << (*it)->getBusstopno_() << "\t";
		}
	}
	Print() << printStream.str() << std::endl;
}

void LaneStats::printAgents(bool copy) const
{
	std::stringstream debugMsgs;
	if (!copy)
	{
		debugMsgs << "Lane: " << lane->getLaneID();
		for (PersonList::const_iterator i = laneAgents.begin(); i != laneAgents.end(); i++)
		{
			debugMsgs << "|" << (*i)->getId();
		}
	}
	else
	{
		debugMsgs << "LaneCopy: " << lane->getLaneID();
		for (PersonList::const_iterator i = laneAgentsCopy.begin(); i != laneAgentsCopy.end(); i++)
		{
			debugMsgs << "|" << (*i)->getId();
		}
	}
	debugMsgs << std::endl;
	Print() << debugMsgs.str();
}

void LaneStats::verifyOrdering()
{
	double distance = -1.0;
	for (PersonList::const_iterator i = laneAgents.begin(); i != laneAgents.end(); i++)
	{
		if (distance >= (*i)->distanceToEndOfSegment)
		{
			std::stringstream debugMsgs;
			debugMsgs << "Invariant violated: Ordering of laneAgents does not reflect ordering w.r.t. distance to end of segment." << "\nSegment: "
					<< lane->getRoadSegment()->getStartEnd() << " length = " << lane->getRoadSegment()->getLaneZeroLength() << "\nLane: " << lane->getLaneID()
					<< "\nCulprit Person: " << (*i)->getId();
			debugMsgs << "\nAgents ";
			for (PersonList::const_iterator j = laneAgents.begin(); j != laneAgents.end(); j++)
			{
				debugMsgs << "|" << (*j)->getId() << "--" << (*j)->distanceToEndOfSegment;
			}
			throw std::runtime_error(debugMsgs.str());
		}
		else
		{
			distance = (*i)->distanceToEndOfSegment;
		}
	}
}

sim_mob::Person* SegmentStats::dequeue(const sim_mob::Person* person, const sim_mob::Lane* lane, bool isQueuingBfrUpdate)
{
	if (!person) { return nullptr; }
	LaneStatsMap::const_iterator laneIt = laneStatsMap.find(lane);
	if(laneIt == laneStatsMap.end())
	{
		throw std::runtime_error("lane not found in segment stats");
	}
	sim_mob::Person* dequeuedPerson = laneIt->second->dequeue(person, isQueuingBfrUpdate);
	if (dequeuedPerson)
	{
		numPersons--; // record removal from segment
	}
	else
	{
		printAgents();
		debugMsgs << "Error: Person " << person->getId() << " was not found in lane " << lane->getLaneID() << std::endl;
		throw std::runtime_error(debugMsgs.str());
	}
	return dequeuedPerson;
}

sim_mob::Person* sim_mob::LaneStats::dequeue(const sim_mob::Person* person, bool isQueuingBfrUpdate)
{
	VehicleBase* vehicle = person->getRole()->getResource();
	if (laneAgents.size() == 0)
	{
		std::stringstream debugMsgs;
		debugMsgs << "Trying to dequeue Person " << person->getId() << " from empty lane." << std::endl;
		throw std::runtime_error(debugMsgs.str());
	}
	sim_mob::Person* p = nullptr;
	if(laneInfinity)
	{
		PersonList::iterator it;
		for (it = laneAgents.begin(); it != laneAgents.end(); it++)
		{
			if ((*it) == person)
			{
				p = (*it);
				it = laneAgents.erase(it); // erase returns the next iterator
				numPersons--; //record removal
				break; //exit loop
			}
		}
	}
	else if (person == laneAgents.front())
	{
		p = laneAgents.front();
		laneAgents.pop_front();
		if (vehicle)
		{
			numPersons--; // record removal
			totalLength = totalLength - vehicle->getLengthCm();
			if (isQueuingBfrUpdate)
			{
				if (queueCount > 0)
				{
					// we have removed a queuing agent
					queueCount--;
					queueLength = queueLength - vehicle->getLengthCm();
				}
				else
				{
					std::stringstream debugMsgs;
					debugMsgs << "Error in dequeue(): queueCount cannot be lesser than 0 in lane." << "\nlane:" << lane->getLaneID() << "|Segment: "
							<< lane->getRoadSegment()->getStartEnd() << "|Person: " << p->getId() << "\nQueuing: " << queueCount << "|Total: " << laneAgents.size()
							<< std::endl;
					Print() << debugMsgs.str();
					throw std::runtime_error(debugMsgs.str());
				}
			}
		}
	}
	return p;
}

} // end of namespace sim_mob
