//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusDriverFacets.hpp"

#include <cstdio>
#include <sstream>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/conflux/LinkStats.hpp"
#include "entities/Person.hpp"
#include "entities/Vehicle.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "logging/Log.hpp"
#include "message/MessageBus.hpp"
#include "message/MT_Message.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

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
}

namespace sim_mob {
namespace medium {

BusDriverBehavior::BusDriverBehavior():
	DriverBehavior(), parentBusDriver(nullptr) {}

BusDriverBehavior::~BusDriverBehavior() {}

void BusDriverBehavior::frame_init() {
	throw std::runtime_error("BusDriverBehavior::frame_init is not implemented yet");
}

void BusDriverBehavior::frame_tick() {
	throw std::runtime_error("BusDriverBehavior::frame_tick is not implemented yet");
}

std::string BusDriverBehavior::frame_tick_output() {
	throw std::runtime_error("BusDriverBehavior::frame_tick_output is not implemented yet");
}


BusDriverMovement::BusDriverMovement():
	DriverMovement(), parentBusDriver(nullptr), busTripId(std::string("NA")) {}

BusDriverMovement::~BusDriverMovement() {}

void BusDriverMovement::frame_init()
{
	bool pathInitialized = initializePath();
	if (pathInitialized)
	{
		Vehicle* newVeh = new Vehicle(Vehicle::BUS, sim_mob::BUS_LENGTH);
		VehicleBase* oldBus = parentBusDriver->getResource();
		safe_delete_item(oldBus);
		parentBusDriver->setResource(newVeh);
		const BusTrip* busTrip = dynamic_cast<const BusTrip*>(*(parentBusDriver->parent->currTripChainItem));
		if(busTrip)
		{
			busTripId = busTrip->tripID;
		}
	}
	else
	{
		parentBusDriver->parent->setToBeRemoved();
	}
}

void BusDriverMovement::frame_tick()
{
	DriverUpdateParams& params = parentBusDriver->getParams();
	if (parentBusDriver->parent->canMoveToNextSegment == Person_MT::GRANTED)
	{
		flowIntoNextLinkIfPossible(params);
	}
	else if (parentBusDriver->parent->canMoveToNextSegment == Person_MT::DENIED)
	{
		if (parentBusDriver->getResource()->isMoving())
		{
			if (currLane)
			{
				if (parentBusDriver->parent->isQueuing)
				{
					moveInQueue();
				}
				else
				{
					addToQueue(); // adds to queue if not already in queue
				}

				params.elapsedSeconds = params.secondsInTick;
				setParentData(params);
			}
		}
		else
		{	//how can it can be denied when serving bus stop??
			params.elapsedSeconds = params.secondsInTick;
			pathMover.setPositionInSegment(0);
			setParentData(params);
		}
		parentBusDriver->parent->canMoveToNextSegment = Person_MT::NONE;
	}

	if (params.elapsedSeconds < params.secondsInTick && !parentBusDriver->getResource()->isMoving())
	{
		// isMoving()==false implies the bus is serving a stop
		if (parentBusDriver->waitingTimeAtbusStop > params.secondsInTick)
		{
			params.elapsedSeconds = params.secondsInTick;
			parentBusDriver->waitingTimeAtbusStop -= params.secondsInTick;
			pathMover.setPositionInSegment(0);
		}
		else
		{
			params.elapsedSeconds += parentBusDriver->waitingTimeAtbusStop;
			parentBusDriver->waitingTimeAtbusStop = 0; //the bus has expired its waiting time

			// There is remaining time, try to move to next segment
			const BusStop* currBusStop = routeTracker.getCurrentStop(); //locally save current stop before moving to next segment
			bool movedToNextSegStat = moveToNextSegment(params);
			if (movedToNextSegStat)
			{
				//moveToNextSegment() calls advance() which inturn may call moveToNextSegment().
				//It is possible that the above call to moveToNextSegment was successful but the bus driver
				//has entered another stop in a downstream segment. In this case, currentStop will be a pointer to the
				//downstream stop that the bus driver entered.
				BusStopAgent* stopAg = BusStopAgent::getBusStopAgentForStop(currBusStop);
				parentBusDriver->closeBusDoors(stopAg); //this handles departure and sets isMoving to true
				if (routeTracker.getCurrentStop())
				{
					if (routeTracker.getCurrentStop() == stopAg->getBusStop())
					{
						//trivial case. the bus driver has simply moved out of current stop
						routeTracker.setCurrentStop(nullptr);
					}
					else
					{
						//the bus driver has moved out of current segment and moved into some other stop in a downstream segment
						//set isMoving back to false because some other stop is being served
						parentBusDriver->getResource()->setMoving(false);
					}
				}
			}
			//else remain in bus stop
		}
	}

	if (!parentBusDriver->parent->requestedNextSegStats && params.elapsedSeconds < params.secondsInTick)
	{
		DriverMovement::frame_tick();
	}
	else if(parentBusDriver->parent->canMoveToNextSegment == Person_MT::NONE)
	{
		// canMoveToNextSegment is set to Granted or Denied only by the conflux
		// If canMoveToNextSegment is not NONE at this point, the bus driver must have remained in VQ
		// Set parent data must not be called if the driver remained in VQ.
		setParentData(params);
	}

	if (params.elapsedSeconds >= params.secondsInTick)
	{
		parentBusDriver->updatePassengers();
	}


/*	//Debug print
	Person_MT* person = parentBusDriver->parent;
	unsigned int segId = (person->getCurrSegStats() ? person->getCurrSegStats()->getRoadSegment()->getRoadSegmentId() : 0);
	uint16_t statsNum = (person->getCurrSegStats() ? person->getCurrSegStats()->getStatsNumberInSegment() : 0);
	const BusStop* nextStop = routeTracker.getNextStop();
	char logbuf[1000];
	sprintf(logbuf, "BD,%u,%s,%s,%u,seg:%u-%u,ln:%u,d:%f,%s,next:%s,q:%c,elpsd:%fs\n",
			person->getId(),
			person->busLine.c_str(),
			busTripId.c_str(),
			parentBusDriver->getParams().now.frame(),
			segId,
			statsNum,
			(person->getCurrLane() ? person->getCurrLane()->getLaneId() : 0),
			 person->distanceToEndOfSegment,
			 (parentBusDriver->getResource()->isMoving()? "road" : "stop"),
			 (nextStop ? nextStop->getStopCode().c_str() : "0"),
			 (person->isQueuing? 'T' : 'F'),
			 params.elapsedSeconds
			);
	person->log(std::string(logbuf));
*/
}

std::string BusDriverMovement::frame_tick_output() {
	DriverUpdateParams &p = parentBusDriver->getParams();
	Person_MT* person = parentBusDriver->getParent();
	//Skip?
	if (pathMover.isPathCompleted() || !person->getCurrSegStats() || !person->getCurrLane()
			|| ConfigManager::GetInstance().FullConfig().using_MPI
			|| ConfigManager::GetInstance().CMakeConfig().OutputDisabled())
	{
		return std::string();
	}

	std::stringstream logout;

	logout << "(\"BusDriver\""
			<<","<<person->getId()
			<<","<<parentBusDriver->getParams().now.frame()
			<<",{"
			<<"\"RoadSegment\":\""<< (person->getCurrSegStats()->getRoadSegment()->getRoadSegmentId())
			<<"\",\"Lane\":\""<<(person->getCurrLane()->getLaneId())
			<<"\",\"DistanceToEndSeg\":\""<<person->distanceToEndOfSegment;
	if(parentBusDriver->getResource()->isMoving()) { logout << "\",\"ServingStop\":\"" << "false"; }
	else { logout << "\",\"ServingStop\":\"" << "true"; }
	if (person->isQueuing) { logout << "\",\"queuing\":\"" << "true"; }
	else { logout << "\",\"queuing\":\"" << "false";}
	logout << "\"})" << std::endl;
	return logout.str();
}

bool BusDriverMovement::initializePath()
{
	Person_MT* person = parentBusDriver->parent;
	if (!person) {
		Print()<<"Parent person of BusDriverMovement is NULL" << std::endl;
		return false;
	}

	//Only initialize if the next path has not been planned for yet.
	if(!person->getNextPathPlanned()){
		//Save local copies of the parent's origin/destination nodes.
		if( person->originNode.type != WayPoint::INVALID){
			parentBusDriver->origin = (*(person->currTripChainItem))->origin;
		}
		if( person->destNode.type != WayPoint::INVALID ){
			parentBusDriver->goal = person->destNode;
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

		//For now, empty paths aren't supporteda.
		if (routeTracker.getRoadSegments().empty()) {
			throw std::runtime_error("Can't initializePath(); path is empty.");
		}
		std::vector<const SegmentStats*> path;
		initSegStatsPath(routeTracker.getRoadSegments(), path);
		if(path.empty()) {
			return false;
		}
		pathMover.setPath(path);
		const SegmentStats* firstSegStat = path.front();
		person->setCurrSegStats(firstSegStat);
		person->setCurrLane(firstSegStat->laneInfinity);
		person->distanceToEndOfSegment = firstSegStat->getLength();
		//routeTracker.printBusRoute(person->getId());
		//pathMover.printPath();
	}

	//to indicate that the path to next activity is already planned
	person->setNextPathPlanned(true);
	return true;

}

const Lane* BusDriverMovement::getBestTargetLane(const SegmentStats* nextSegStats, const SegmentStats* nextToNextSegStats)
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
		const Lane* minLane = nullptr;
		double minQueueLength = std::numeric_limits<double>::max();
		double minLength = std::numeric_limits<double>::max();
		double que = 0.0;
		double total = 0.0;

		const Link* nextLink = getNextLinkForLaneChoice(nextSegStats);
		const std::vector<Lane*>& lanes = nextSegStats->getRoadSegment()->getLanes();
		for (vector<Lane* >::const_iterator lnIt=lanes.begin(); lnIt!=lanes.end(); ++lnIt)
		{
			const Lane* lane = *lnIt;
			if (!lane->isPedestrianLane())
			{
				if(nextToNextSegStats
						&& !isConnectedToNextSeg(lane, nextToNextSegStats->getRoadSegment())
						&& nextLink
						&& !nextSegStats->isConnectedToDownstreamLink(nextLink, lane))
				{ continue; }
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
			for (vector<Lane* >::const_iterator lnIt=lanes.begin(); lnIt!=lanes.end(); ++lnIt)
			{
				if (!((*lnIt)->isPedestrianLane()))
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

void BusDriverMovement::departFromCurrentStop()
{
	if (!parentBusDriver->getResource()->isMoving())
	{
		const BusStop* stop = routeTracker.getCurrentStop();
		if(!stop)
		{
			throw std::runtime_error("Invalid bus driver state. Current stop is NULL while bus is serving stop");
		}
		BusStopAgent* stopAg = BusStopAgent::getBusStopAgentForStop(stop);
		parentBusDriver->closeBusDoors(stopAg);
		routeTracker.setCurrentStop(nullptr);
	}
}

void BusDriverMovement::flowIntoNextLinkIfPossible(DriverUpdateParams& params)
{
	//This function gets called for 2 cases.
	//1. Driver is added to virtual queue
	//2. Driver is in previous segment trying to add to the next
	const SegmentStats* nextSegStats = pathMover.getNextSegStats(false);
	const SegmentStats* nextToNextSegStats = pathMover.getSecondSegStatsAhead();
	const Lane* laneInNextSegment = getBestTargetLane(nextSegStats, nextToNextSegStats);

	double departTime = getLastAccept(laneInNextSegment, nextSegStats);
/*	if(!nextSegStats->isShortSegment())
	{
		if(nextSegStats->hasQueue())
		{
			departTime += getAcceptRate(laneInNextSegment, nextSegStats); //in seconds
		}
		else
		{
			departTime += (PASSENGER_CAR_UNIT / (nextSegStats->getNumVehicleLanes() * nextSegStats->getSegSpeed(true)));
		}
	}
*/
	params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (params.now.ms()/1000.0)); //in seconds

	const Link* nextLink = getNextLinkForLaneChoice(nextSegStats);
	if (canGoToNextRdSeg(params, nextSegStats, nextLink))
	{
		if (parentBusDriver->getResource()->isMoving())
		{
			if (isQueuing)
			{
				removeFromQueue();
			}
		}
		else
		{
			departFromCurrentStop();
		}

		currLane = laneInNextSegment;
		pathMover.advanceInPath();
		pathMover.setPositionInSegment(nextSegStats->getLength());

		//todo: consider supplying milliseconds to be consistent with short-term
		double linkExitTimeSec = params.elapsedSeconds + (params.now.ms()/1000.0);
		//set Link Travel time for previous link
		const SegmentStats* prevSegStats = pathMover.getPrevSegStats(false);
		if (prevSegStats)
		{
			// update link travel times
			updateLinkTravelTimes(prevSegStats, linkExitTimeSec);

			// update link stats
			updateLinkStats(prevSegStats);

			// update road segment screenline counts
			updateScreenlineCounts(prevSegStats, linkExitTimeSec);
		}
		setLastAccept(currLane, linkExitTimeSec, nextSegStats);

		if (!parentBusDriver->getResource()->isMoving())
		{
			departFromCurrentStop();
		}

		setParentData(params);
		parentBusDriver->parent->canMoveToNextSegment = Person_MT::NONE;
	}
	else
	{
		if (parentBusDriver->getResource()->isMoving())
		{
			//Person is in previous segment (should be added to queue if canGoTo failed)
			if (pathMover.getCurrSegStats() == parentBusDriver->parent->getCurrSegStats())
			{
				if (currLane)
				{
					if (parentBusDriver->parent->isQueuing)
					{
						moveInQueue();
					}
					else
					{
						addToQueue(); // adds to queue if not already in queue
					}
					parentBusDriver->parent->canMoveToNextSegment = Person_MT::NONE; // so that advance() and setParentData() is called subsequently
				}
			}
			else if (pathMover.getNextSegStats(false) == parentBusDriver->parent->getCurrSegStats())
			{
				//Person is in virtual queue (should remain in virtual queues if canGoTo failed)
				//do nothing
			}
			else
			{
				DebugStream << "Driver " << parentBusDriver->parent->getId() << "was neither in virtual queue nor in previous segment!" << "\ndriver| segment: "
						<< pathMover.getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|id: "
						<< pathMover.getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|lane: " << currLane->getLaneId() << "\nPerson| segment: "
						<< parentBusDriver->parent->getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|id: "
						<< parentBusDriver->parent->getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|lane: "
						<< (parentBusDriver->parent->getCurrLane() ? parentBusDriver->parent->getCurrLane()->getLaneId() : 0) << std::endl;

				throw ::std::runtime_error(DebugStream.str());
			}
			params.elapsedSeconds = params.secondsInTick;
			parentBusDriver->parent->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
		}
		else
		{
			//the bus driver is currently serving a stop
			params.elapsedSeconds = params.secondsInTick; //remain in bus stop
			parentBusDriver->parent->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
			parentBusDriver->parent->canMoveToNextSegment = Person_MT::NONE; // so that in the next tick, flowIntoNextLinkIfPossible() is not called in the next tick without requesting for permission again
		}
	}
}

bool BusDriverMovement::moveToNextSegment(DriverUpdateParams& params)
{
	const SegmentStats* currSegStat = pathMover.getCurrSegStats();
	const BusStop* nextStop = routeTracker.getNextStop();
	if (nextStop && currSegStat->hasBusStop(nextStop))
	{
		BusStopAgent* stopAg = BusStopAgent::getBusStopAgentForStop(nextStop);
		if (stopAg)
		{
			if (stopAg->canAccommodate(parentBusDriver->getResource()->getLengthInM()))
			{
				if (parentBusDriver->getResource()->isMoving())
				{
					if (isQueuing)
					{
						removeFromQueue();
					}
				}
				else
				{
					departFromCurrentStop();
				}

				DailyTime startTm = ConfigManager::GetInstance().FullConfig().simStartTime();
				DailyTime current(params.now.ms() + converToMilliseconds(params.elapsedSeconds) + startTm.getValue());
				routeTracker.setCurrentStop(nextStop);
				routeTracker.updateNextStop();
				parentBusDriver->openBusDoors(current.getStrRepr(), stopAg);
				double remainingTime = params.secondsInTick - params.elapsedSeconds;
				if (parentBusDriver->waitingTimeAtbusStop > remainingTime)
				{
					parentBusDriver->waitingTimeAtbusStop -= remainingTime;
					params.elapsedSeconds = params.secondsInTick;
				}
				else
				{
					params.elapsedSeconds += parentBusDriver->waitingTimeAtbusStop;
					parentBusDriver->waitingTimeAtbusStop = 0;
					// There is remaining time, try to move to next segment
					bool movedToNextSegStat = moveToNextSegment(params);
					if (movedToNextSegStat)
					{
						//moveToNextSegment() calls advance() which inturn may call moveToNextSegment().
						//It is possible that the above call to moveToNextSegment was successful but the bus driver
						//has entered another stop in a downstream segment. In this case, currentStop will be a pointer to the
						//downstream stop that the bus driver entered.
						parentBusDriver->closeBusDoors(stopAg); //this handles departure and sets isMoving to true
						if (routeTracker.getCurrentStop())
						{
							if (routeTracker.getCurrentStop() == stopAg->getBusStop())
							{
								//trivial case. the bus driver has simply moved out of current stop
								routeTracker.setCurrentStop(nullptr);
							}
							else
							{
								//the bus driver has moved out of current segment and moved into some other stop in a downstream segment
								//set isMoving back to false because some other stop is being served
								parentBusDriver->getResource()->setMoving(false);
							}
						}
					}
					//else remain in bus stop
					return movedToNextSegStat;
				}
			}
			else
			{
				if(parentBusDriver->getResource()->isMoving())
				{
					if (isQueuing)
					{
						moveInQueue();
					}
					else
					{
						addToQueue();
					}
				}
				params.elapsedSeconds = params.secondsInTick;
				parentBusDriver->parent->setRemainingTimeThisTick(0.0);
			}
		}
		else
		{
			std::stringstream errorStrm;
			errorStrm << "BusStopAgent not found for stop: " << nextStop->getStopCode() << "|SegmentStats: " << currSegStat << "|position: "
					<< currSegStat->getStatsNumberInSegment() << "|num. stops: " << currSegStat->getNumStops() << std::endl;
			throw std::runtime_error(errorStrm.str());
		}
		return false;
	}
	else
	{
		bool res = false;
		bool isNewLinkNext = (!pathMover.hasNextSegStats(true) && pathMover.hasNextSegStats(false));
		const SegmentStats* currSegStat = pathMover.getCurrSegStats();
		const SegmentStats* nxtSegStat = pathMover.getNextSegStats(!isNewLinkNext);

		//currently the best place to call a handler indicating 'Done' with segment.
		const RoadSegment *curRs = (*(pathMover.getCurrSegStats())).getRoadSegment();
		//Although the name of the method suggests segment change, it is actually segStat change. so we check again!
		const RoadSegment *nxtRs = (nxtSegStat ? nxtSegStat->getRoadSegment() : nullptr);

		if (curRs && curRs != nxtRs)
		{
			onSegmentCompleted(curRs, nxtRs);
		}

		if (isNewLinkNext)
		{
			onLinkCompleted(curRs->getParentLink(), (nxtRs ? nxtRs->getParentLink() : nullptr));
		}

		//reset these local variables in case path has been changed in onLinkCompleted
		isNewLinkNext = (!pathMover.hasNextSegStats(true) && pathMover.hasNextSegStats(false));
		currSegStat = pathMover.getCurrSegStats();
		nxtSegStat = pathMover.getNextSegStats(!isNewLinkNext);

		if (!nxtSegStat)
		{
			//vehicle is done
			pathMover.advanceInPath();
			if (pathMover.isPathCompleted())
			{
				const Link* currLink = currSegStat->getRoadSegment()->getParentLink();
				double linkExitTime = params.elapsedSeconds + (params.now.ms()/1000.0);
				parentBusDriver->parent->currLinkTravelStats.finalize(currLink, linkExitTime, nullptr);
				TravelTimeManager::getInstance()->addTravelTime(parentBusDriver->parent->currLinkTravelStats); //in seconds
				currSegStat->getParentConflux()->setLinkTravelTimes(linkExitTime, currLink);
				parentBusDriver->parent->currLinkTravelStats.reset();
				currSegStat->getParentConflux()->getLinkStats(currLink).removeEntitiy(parentBusDriver->parent);
				setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat) - 1), currSegStat);
				currLane = nullptr;
				parentBusDriver->parent->setToBeRemoved();
			}
			params.elapsedSeconds = params.secondsInTick;
			return true;
		}

		if (isNewLinkNext)
		{
			parentBusDriver->parent->requestedNextSegStats = nxtSegStat;
			parentBusDriver->parent->canMoveToNextSegment = Person_MT::NONE;
			return false; // return whenever a new link is to be entered. Seek permission from Conflux.
		}

		const SegmentStats* nextToNextSegStat = pathMover.getSecondSegStatsAhead();
		const Lane* laneInNextSegment = getBestTargetLane(nxtSegStat, nextToNextSegStat);

		double departTime = getLastAccept(laneInNextSegment, nxtSegStat);
/*		if(!nxtSegStat->isShortSegment())
		{
			if(nxtSegStat->hasQueue())
			{
				departTime += getAcceptRate(laneInNextSegment, nxtSegStat); //in seconds
			}
			else
			{
				departTime += (PASSENGER_CAR_UNIT / (nxtSegStat->getNumVehicleLanes() * nxtSegStat->getSegSpeed(true)));
			}
		}
*/
		params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (params.now.ms()/1000.0)); //in seconds

		const Link* nextLink = getNextLinkForLaneChoice(nxtSegStat);
		if (canGoToNextRdSeg(params, nxtSegStat, nextLink))
		{
			if (isQueuing)
			{
				removeFromQueue();
			}

			setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat) - 1), currSegStat); // decrement from the currLane before updating it
			currLane = laneInNextSegment;
			pathMover.advanceInPath();
			pathMover.setPositionInSegment(nxtSegStat->getLength());
			double segExitTimeSec = params.elapsedSeconds + (params.now.ms()/1000.0);
			setLastAccept(currLane, segExitTimeSec, nxtSegStat);

			const SegmentStats* prevSegStats = pathMover.getPrevSegStats(true); //previous segment is in the same link
			if (prevSegStats
					&& prevSegStats->getRoadSegment() != pathMover.getCurrSegStats()->getRoadSegment())
			{
				// update road segment travel times
				updateScreenlineCounts(prevSegStats, segExitTimeSec);
			}

			res = true;
			parentBusDriver->getResource()->setMoving(true);//set moving is set to true here explicitly because the BD could try to move to next segement from within advance and we want the correct moving status there
			advance(params);
		}
		else
		{
			if(parentBusDriver->getResource()->isMoving())
			{
				if (isQueuing)
				{
					moveInQueue();
				}
				else
				{
					addToQueue();
				}
			}

			params.elapsedSeconds = params.secondsInTick;
			parentBusDriver->parent->setRemainingTimeThisTick(0.0);
		}
		return res;
	}
}

BusRouteTracker::BusRouteTracker(const BusRouteInfo& routeInfo) :
		BusRouteInfo(routeInfo), nextStopIt(busStopList.begin()), currentBusStop(nullptr)
{
}

BusRouteTracker::BusRouteTracker(const BusRouteTracker& copySource) :
		BusRouteInfo(copySource), nextStopIt(busStopList.begin()), currentBusStop(nullptr)
{
}

BusRouteTracker& BusRouteTracker::operator=(const BusRouteTracker& rhsTracker)
{
	if (&rhsTracker != this)
	{
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

const BusStop* BusRouteTracker::getCurrentStop() const
{
	return currentBusStop;
}

void BusRouteTracker::setCurrentStop(const BusStop* currStop)
{
	currentBusStop = currStop;
}

void BusRouteTracker::printBusRoute(unsigned int personId)
{
	const vector<const RoadSegment*>& rsPath = getRoadSegments();
	std::stringstream printStrm;
	printStrm << "personId: " << personId << "|bus line: "<< this->busRouteId << std::endl;
	printStrm << "segments in bus trip: "<< rsPath.size() << "\nsegments: ";
	for (vector<const RoadSegment*>::const_iterator it = rsPath.begin(); it != rsPath.end(); it++)
	{
		const RoadSegment* rdSeg = *it;
		printStrm << rdSeg->getRoadSegmentId() << "|";
	}
	printStrm << std::endl;
	const vector<const BusStop*>& stops = this->getBusStops();
	printStrm << "stops in bus trip: "<< stops.size() << "\nstops: ";
	for (vector<const BusStop*>::const_iterator it = stops.begin(); it != stops.end(); it++)
	{
		printStrm << (*it)->getStopCode() << "|";
	}
	printStrm << std::endl;
	Print() << printStrm.str();
}
}
}
