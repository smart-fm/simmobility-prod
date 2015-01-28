//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusDriverFacets.hpp"

#include <sstream>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/Person.hpp"
#include "entities/Vehicle.hpp"
#include "geospatial/RoadSegment.hpp"
#include "logging/Log.hpp"
#include "message/MessageBus.hpp"
#include "message/MT_Message.hpp"

using namespace sim_mob;

using std::vector;
using std::endl;

namespace {

/**
 * converts time from milli-seconds to seconds
 */
inline double converToSeconds(uint32_t timeInMs) {
	return (timeInMs/1000.0);
}

/**
 * converts time from  seconds to milli-seconds
 */
inline unsigned int converToMilliseconds(double timeInMs) {
	return (timeInMs*1000.0);
}

/**
 * length of a bus is hard coded to 3 times the PCU for now.
 * TODO: this must be made configurable.
 */
const double BUS_LENGTH = 3 * sim_mob::PASSENGER_CAR_UNIT; // 3 times PASSENGER_CAR_UNIT
}

namespace sim_mob {
namespace medium {

sim_mob::medium::BusDriverBehavior::BusDriverBehavior(sim_mob::Person* parentAgent):
	DriverBehavior(parentAgent), parentBusDriver(nullptr) {}

sim_mob::medium::BusDriverBehavior::~BusDriverBehavior() {}

void sim_mob::medium::BusDriverBehavior::frame_init() {
	throw std::runtime_error("BusDriverBehavior::frame_init is not implemented yet");
}

void sim_mob::medium::BusDriverBehavior::frame_tick() {
	throw std::runtime_error("BusDriverBehavior::frame_tick is not implemented yet");
}

void sim_mob::medium::BusDriverBehavior::frame_tick_output() {
	throw std::runtime_error("BusDriverBehavior::frame_tick_output is not implemented yet");
}


sim_mob::medium::BusDriverMovement::BusDriverMovement(sim_mob::Person* parentAgent):
	DriverMovement(parentAgent), parentBusDriver(nullptr) {}

sim_mob::medium::BusDriverMovement::~BusDriverMovement() {}

void sim_mob::medium::BusDriverMovement::frame_init() {
	bool pathInitialized = initializePath();
	if (pathInitialized) {
		Vehicle* newVeh = new Vehicle(Vehicle::BUS, BUS_LENGTH);
		VehicleBase* oldBus = parentBusDriver->getResource();
		safe_delete_item(oldBus);
		parentBusDriver->setResource(newVeh);
	}
}

void BusDriverMovement::frame_tick() {
	sim_mob::medium::DriverUpdateParams& params = parentBusDriver->getParams();
	if(!parentBusDriver->getResource()->isMoving()) {
		// isMoving()==false implies the bus is serving a stop
		if (parentBusDriver->waitingTimeAtbusStop > params.secondsInTick) {
			params.elapsedSeconds = params.secondsInTick;
			parentBusDriver->waitingTimeAtbusStop -= params.secondsInTick;
			pathMover.setPositionInSegment(0);
		}
		else {
			params.elapsedSeconds = params.elapsedSeconds + parentBusDriver->waitingTimeAtbusStop;
			parentBusDriver->waitingTimeAtbusStop = 0.0;
			//the bus has expired its waiting time
			//send bus departure message
			const BusStop* stop = routeTracker.getNextStop();
			BusStopAgent* stopAg = BusStopAgent::findBusStopAgentByBusStop(stop);
			const sim_mob::SegmentStats* currSegStat = pathMover.getCurrSegStats();
			parentBusDriver->closeBusDoors(stopAg);
			routeTracker.updateNextStop();
			DriverMovement::moveToNextSegment(params);
		}
	}
	if(params.elapsedSeconds < params.secondsInTick)
	{
		DriverMovement::frame_tick();
	}
	else
	{
		setParentData(params);
	}
	std::stringstream logout;
	sim_mob::Person* person = getParent();
	unsigned int segId = (person->getCurrSegStats()? person->getCurrSegStats()->getRoadSegment()->getSegmentAimsunId() : 0 );
	uint16_t statsNum = (person->getCurrSegStats()? person->getCurrSegStats()->getStatsNumberInSegment() : 0);
	logout << "(BusDriver"
			<<","<<person->getId()
			<<","<<parentBusDriver->getParams().now.frame()
			<<",{"
			<<"RoadSegment:"<< segId
			<<",StatsNum:"<< statsNum
			<<",Lane:"<<(person->getCurrLane()? person->getCurrLane()->getLaneID() : 0)
			<<",DistanceToEndSeg:"<<person->distanceToEndOfSegment;

	if(parentBusDriver->getResource()->isMoving()) { logout << ",ServingStop:" << "false"; }
	else { logout << ",ServingStop:" << "true"; }

	if (person->isQueuing) { logout << ",queuing:" << "true"; }
	else { logout << ",queuing:" << "false";}
	logout << "})" << std::endl;
	Print()<<logout.str();
}

void sim_mob::medium::BusDriverMovement::frame_tick_output() {
	sim_mob::medium::DriverUpdateParams &p = parentBusDriver->getParams();
	//Skip?
	if (pathMover.isPathCompleted()
			|| ConfigManager::GetInstance().FullConfig().using_MPI
			|| ConfigManager::GetInstance().CMakeConfig().OutputDisabled()) {
		return;
	}

	std::stringstream logout;
	sim_mob::Person* person = getParent();
	logout << "(\"BusDriver\""
			<<","<<person->getId()
			<<","<<parentBusDriver->getParams().now.frame()
			<<",{"
			<<"\"RoadSegment\":\""<< (person->getCurrSegStats()->getRoadSegment()->getId())
			<<"\",\"Lane\":\""<<(person->getCurrLane()->getLaneID())
			<<"\",\"UpNode\":\""<<(person->getCurrSegStats()->getRoadSegment()->getStart()->getID())
			<<"\",\"DistanceToEndSeg\":\""<<person->distanceToEndOfSegment;
	if(parentBusDriver->getResource()->isMoving()) { logout << "\",\"ServingStop\":\"" << "false"; }
	else { logout << "\",\"ServingStop\":\"" << "true"; }
	if (person->isQueuing) { logout << "\",\"queuing\":\"" << "true"; }
	else { logout << "\",\"queuing\":\"" << "false";}
	logout << "\"})" << std::endl;
	Print()<<logout.str();
	LogOut(logout.str());
}

bool sim_mob::medium::BusDriverMovement::initializePath()
{
	sim_mob::Person* person = getParent();
	if (!person) {
		Print()<<"Parent person of BusDriverMovement is NULL" << std::endl;
		return false;
	}

	//Only initialize if the next path has not been planned for yet.
	if(!person->getNextPathPlanned()){
		//Save local copies of the parent's origin/destination nodes.
		if( person->originNode.type_ != WayPoint::INVALID){
			parentBusDriver->origin.node = person->originNode.node_;
			parentBusDriver->origin.point = parentBusDriver->origin.node->location;
		}
		if( person->destNode.type_ != WayPoint::INVALID ){
			parentBusDriver->goal.node = person->destNode.node_;
			parentBusDriver->goal.point = parentBusDriver->goal.node->location;
		}

		const BusTrip* bustrip = dynamic_cast<const BusTrip*>(*(person->currTripChainItem));
		if (!bustrip) {
			Print()<< "bustrip is null"<<std::endl;
		}
		else if ((*(person->currTripChainItem))->itemType == TripChainItem::IT_BUSTRIP) {
			routeTracker = BusRouteTracker(bustrip->getBusRouteInfo()); //Calls the constructor and the assignment operator overload defined in this class. Copy-constructor is not called.
		} else {
			Print() << "BusTrip path not initialized because it is not a bustrip, (*(person->currTripChainItem))->itemType = "
					<< (*(person->currTripChainItem))->itemType << std::endl;
		}

		//For now, empty paths aren't supported.
		if (routeTracker.getRoadSegments().empty()) {
			throw std::runtime_error("Can't initializePath(); path is empty.");
		}
		std::vector<const sim_mob::SegmentStats*> path;
		initSegStatsPath(routeTracker.getRoadSegments(), path);
		if(path.empty()) {
			return false;
		}
		pathMover.setPath(path);
		const sim_mob::SegmentStats* firstSegStat = path.front();
		person->setCurrSegStats(firstSegStat);
		person->setCurrLane(firstSegStat->laneInfinity);
		person->distanceToEndOfSegment = firstSegStat->getLength();
		routeTracker.printBusRoute(person->getId());
		pathMover.printPath();
	}

	//to indicate that the path to next activity is already planned
	person->setNextPathPlanned(true);
	return true;

}

const sim_mob::Lane* BusDriverMovement::getBestTargetLane(
		const sim_mob::SegmentStats* nextSegStats,
		const SegmentStats* nextToNextSegStats)
{
	if(!nextSegStats) { return nullptr; }
	const BusStop* nextStop = routeTracker.getNextStop();
	/* Even if there is a bus lane in the next segment stats, the bus can choose
	 * not to enter it if it does not have to serve the stop in the next segment
	 * stats. So, lane selection is purely based on whether the next stop is in
	 * the next seg stats.
	 */
	if(nextStop && nextSegStats->hasBusStop(nextStop))
	{
		return nextSegStats->getOutermostLane();
	}
	else
	{
		const sim_mob::Lane* minLane = nullptr;
		double minQueueLength = std::numeric_limits<double>::max();
		double minLength = std::numeric_limits<double>::max();
		double que = 0.0;
		double total = 0.0;

		const std::vector<sim_mob::Lane*>& lanes = nextSegStats->getRoadSegment()->getLanes();
		for (vector<sim_mob::Lane* >::const_iterator lnIt=lanes.begin(); lnIt!=lanes.end(); ++lnIt)
		{
			if (!((*lnIt)->is_pedestrian_lane()))
			{
				const Lane* lane = *lnIt;
				if(nextToNextSegStats && !isConnectedToNextSeg(lane, nextToNextSegStats->getRoadSegment()))
				{
					continue;
				}
				total = nextSegStats->getLaneTotalVehicleLength(lane);
				que = nextSegStats->getLaneQueueLength(lane);
				if (minLength > total)
				{
					//if total length of vehicles is less than current minLength
					minLength = total;
					minQueueLength = que;
					minLane = lane;
				}
				else if (minLength == total)
				{
					//if total length of vehicles is equal to current minLength
					if (minQueueLength > que)
					{
						//and if the queue length is less than current minQueueLength
						minQueueLength = que;
						minLane = lane;
					}
				}
			}
		}

		if(!minLane)
		{
			//throw std::runtime_error("best target lane was not set!");
			//TODO: if minLane is null, there is probably no lane connection from any lane in next segment stats to
			// the lanes in the nextToNextSegmentStats. The code in this block is a hack to avoid errors due to this reason.
			//This code must be removed and an error must be thrown here in future.
			for (vector<sim_mob::Lane* >::const_iterator lnIt=lanes.begin(); lnIt!=lanes.end(); ++lnIt)
			{
				if (!((*lnIt)->is_pedestrian_lane()))
				{
					const Lane* lane = *lnIt;
					total = nextSegStats->getLaneTotalVehicleLength(lane);
					que = nextSegStats->getLaneQueueLength(lane);
					if (minLength > total)
					{
						//if total length of vehicles is less than current minLength
						minLength = total;
						minQueueLength = que;
						minLane = lane;
					}
					else if (minLength == total)
					{
						//if total length of vehicles is equal to current minLength
						if (minQueueLength > que)
						{
							//and if the queue length is less than current minQueueLength
							minQueueLength = que;
							minLane = lane;
						}
					}
				}
			}
		}
		return minLane;
	}
}

bool BusDriverMovement::moveToNextSegment(DriverUpdateParams& params)
{
	const sim_mob::SegmentStats* currSegStat = pathMover.getCurrSegStats();
	const BusStop* nextStop = routeTracker.getNextStop();
	if(nextStop && currSegStat->hasBusStop(nextStop))
	{
		BusStopAgent* stopAg = BusStopAgent::findBusStopAgentByBusStop(nextStop);
		if(stopAg)
		{
			if(stopAg->canAccommodate(parentBusDriver->getResource()->getLengthCm()))
			{
				if(isQueuing)
				{
					removeFromQueue();
				}
				DailyTime startTm = ConfigManager::GetInstance().FullConfig().simStartTime();
				DailyTime current(params.now.ms()+converToMilliseconds(params.elapsedSeconds)+startTm.getValue());
				parentBusDriver->openBusDoors(current.toString(), stopAg);
				double remainingTime = params.secondsInTick - params.elapsedSeconds;
				if(parentBusDriver->waitingTimeAtbusStop > remainingTime) {
					parentBusDriver->waitingTimeAtbusStop -= remainingTime;
					params.elapsedSeconds = params.secondsInTick;
				}
				else {
					params.elapsedSeconds += parentBusDriver->waitingTimeAtbusStop;
					parentBusDriver->waitingTimeAtbusStop = 0;
					parentBusDriver->closeBusDoors(stopAg);
					routeTracker.updateNextStop();
					// There is remaining time, try to move to next segment
					return DriverMovement::moveToNextSegment(params);
				}
			}
			else
			{
				if (isQueuing)
				{
					moveInQueue();
				}
				else
				{
					addToQueue();
				}
				params.elapsedSeconds = params.secondsInTick;
				getParent()->setRemainingTimeThisTick(0.0);
			}
		}
		else
		{
			std::stringstream errorStrm;
			errorStrm << "BusStopAgent not found for stop: " << nextStop->getBusstopno_()
					<< "|SegmentStats: " << currSegStat
					<< "|position: " << currSegStat->getStatsNumberInSegment()
					<< "|num. stops: " << currSegStat->getNumStops()
					<< std::endl;
			throw std::runtime_error(errorStrm.str());
		}
		return false;
	}
	else
	{
		return DriverMovement::moveToNextSegment(params);
	}
}

BusRouteTracker::BusRouteTracker(const BusRouteInfo& routeInfo)
: BusRouteInfo(routeInfo), nextStopIt(busStopList.begin()) {}

BusRouteTracker::BusRouteTracker(const BusRouteTracker& copySource)
: BusRouteInfo(copySource), nextStopIt(busStopList.begin()) {}

BusRouteTracker& BusRouteTracker::operator=(const BusRouteTracker& rhsTracker)
{
	if(&rhsTracker != this) {
		busRouteId = rhsTracker.busRouteId;
		roadSegmentList = rhsTracker.roadSegmentList;
		busStopList = rhsTracker.busStopList;
		nextStopIt = busStopList.begin();
		std::advance(nextStopIt, (rhsTracker.nextStopIt - rhsTracker.busStopList.begin()));
	}
	return *this;
}

const BusStop* BusRouteTracker::getNextStop() const
{
	if(nextStopIt==busStopList.end()) { return nullptr;	}
	return *nextStopIt;
}

void BusRouteTracker::updateNextStop()
{
	if(nextStopIt==busStopList.end()) { return;	}
	nextStopIt++;
}

void BusRouteTracker::printBusRoute(unsigned int personId)
{
	const vector<const sim_mob::RoadSegment*>& rsPath = getRoadSegments();
	std::stringstream printStrm;
	printStrm << "personId: " << personId << "|bus line: "<< this->busRouteId << std::endl;
	printStrm << "segments in bus trip: "<< rsPath.size() << "\nsegments: ";
	for (vector<const sim_mob::RoadSegment*>::const_iterator it = rsPath.begin(); it != rsPath.end(); it++)
	{
		const sim_mob::RoadSegment* rdSeg = *it;
		printStrm << rdSeg->getSegmentAimsunId() << "|";
	}
	printStrm << std::endl;
	const vector<const sim_mob::BusStop*>& stops = this->getBusStops();
	printStrm << "stops in bus trip: "<< stops.size() << "\nstops: ";
	for (vector<const sim_mob::BusStop*>::const_iterator it = stops.begin(); it != stops.end(); it++)
	{
		printStrm << (*it)->busstopno_ << "|";
	}
	printStrm << std::endl;
	Print() << printStrm.str();
}
}
}
