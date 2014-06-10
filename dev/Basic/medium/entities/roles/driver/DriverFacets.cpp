//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverFacets.hpp"
#include <cmath>
#include <ostream>
#include <algorithm>

#include "buffering/BufferedDataManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Person.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/Vehicle.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/PathSetManager.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"

#include "logging/Log.hpp"

#include "partitions/PartitionManager.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"

#include "util/DebugFlags.hpp"
using namespace sim_mob;
using namespace sim_mob::medium;
using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;

namespace {
/**
 * accepts a list of WayPoint-s and returns a list of SegmentStats* corresponding
 * to RoadSegment* in the list of WayPoint.
 */
void initSegStatsPath(vector<sim_mob::WayPoint>& wpPath,
		vector<const sim_mob::SegmentStats*>& ssPath) {
	for (vector<sim_mob::WayPoint>::iterator it = wpPath.begin();
			it != wpPath.end(); it++) {
		if (it->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* rdSeg = it->roadSegment_;
			const vector<sim_mob::SegmentStats*>& statsInSegment =
					rdSeg->getParentConflux()->findSegStats(rdSeg);
			ssPath.insert(ssPath.end(), statsInSegment.begin(), statsInSegment.end());
		}
	}
}

/**
 * converts time from milli-seconds to seconds
 */
inline double converToSeconds(uint32_t timeInMs) {
	return (timeInMs/1000.0);
}

/**an infinitesimal double to avoid rounding issues*/
const double INFINITESIMAL_DOUBLE = 0.0001;
}

namespace sim_mob {
namespace medium {

DriverBehavior::DriverBehavior(sim_mob::Person* parentAgent):
	BehaviorFacet(parentAgent), parentDriver(nullptr) {}

DriverBehavior::~DriverBehavior() {}

void DriverBehavior::frame_init() {
	throw std::runtime_error("DriverBehavior::frame_init is not implemented yet");
}

void DriverBehavior::frame_tick() {
	throw std::runtime_error("DriverBehavior::frame_tick is not implemented yet");
}

void DriverBehavior::frame_tick_output() {
	throw std::runtime_error("DriverBehavior::frame_tick_output is not implemented yet");
}

sim_mob::medium::Driver* sim_mob::medium::DriverBehavior::getParentDriver() {
	return parentDriver;
}

sim_mob::medium::DriverMovement::DriverMovement(sim_mob::Person* parentAgent):
	MovementFacet(parentAgent), parentDriver(nullptr), currLane(nullptr),
	laneInNextSegment(nullptr), isQueuing(false)
{}


sim_mob::medium::DriverMovement::~DriverMovement() {}

void sim_mob::medium::DriverMovement::frame_init() {
	bool pathInitialized = initializePath();
	if (pathInitialized) {
		Vehicle* newVehicle = new Vehicle(Vehicle::CAR, PASSENGER_CAR_UNIT);
		VehicleBase* oldVehicle = parentDriver->getResource();
		safe_delete_item(oldVehicle);
		parentDriver->setResource(newVehicle);
	}
	else{
		getParent()->setToBeRemoved();
	}
}

void sim_mob::medium::DriverMovement::frame_tick() {
	sim_mob::medium::DriverUpdateParams& params = parentDriver->getParams();
	const sim_mob::SegmentStats* currSegStats = pathMover.getCurrSegStats();
	if(!currSegStats) {
		//if currSegstats is NULL, either the driver did not find a path to his
		//destination or his path is completed. Either way, we remove this
		//person from the simulation.
		getParent()->setToBeRemoved();
		return;
	}
	else if (currSegStats == getParent()->getCurrSegStats())
	{
		//the vehicle will be in lane infinity before it starts.
		//set origin will move it to the correct lane
		if (getParent()->getCurrLane() == currSegStats->laneInfinity) {
			setOrigin(params);
		}
	}
	//canMoveToNextSegment is GRANTED/DENIED only when this driver had previously
	//requested permission to move to the next segment. This request is made
	//only when the driver has reached the end of the current link
	if(getParent()->canMoveToNextSegment == Person::GRANTED) {
		flowIntoNextLinkIfPossible(params);
	}
	else if (getParent()->canMoveToNextSegment == Person::DENIED){
		if(currLane) {
			if(getParent()->isQueuing) {
				moveInQueue();
			}
			else {
				addToQueue(); // adds to queue if not already in queue
			}

			params.elapsedSeconds = params.secondsInTick;
			getParent()->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
			setParentData(params);
		}
	}
	//if driver is still in lane infinity (currLane is null),
	//he shouldn't be advanced
	if (currLane && getParent()->canMoveToNextSegment == Person::NONE) {
		advance(params);
		setParentData(params);
	}
}

void sim_mob::medium::DriverMovement::frame_tick_output() {
	const sim_mob::medium::DriverUpdateParams& params = parentDriver->getParams();
	if (pathMover.isPathCompleted()
			|| ConfigManager::GetInstance().FullConfig().using_MPI
			|| ConfigManager::GetInstance().CMakeConfig().OutputDisabled()) {
		return;
	}

	std::stringstream logout;
	logout << "(\"Driver\""
			<<","<<getParent()->getId()
			<<","<<params.now.frame()
			<<",{"
			<<"\"RoadSegment\":\""<< (getParent()->getCurrSegStats()->getRoadSegment()->getSegmentID())
			<<"\",\"Lane\":\""<<((getParent()->getCurrLane())? getParent()->getCurrLane()->getLaneID():0)
			<<"\",\"Segment\":\""<<(getParent()->getCurrSegStats()->getRoadSegment()->getStartEnd())
			<<"\",\"DistanceToEndSeg\":\""<<getParent()->distanceToEndOfSegment;
	if (this->getParent()->isQueuing) {
			logout << "\",\"queuing\":\"" << "true";
	} else {
			logout << "\",\"queuing\":\"" << "false";
	}
	logout << "\"})" << std::endl;

	LogOut(logout.str());
}

bool sim_mob::medium::DriverMovement::initializePath() {
	//Only initialize if the next path has not been planned for yet.
	sim_mob::Person* person = getParent();
	if(!person->getNextPathPlanned()){
		//Save local copies of the parent's origin/destination nodes.
		parentDriver->origin.node = person->originNode.node_;
		parentDriver->origin.point = parentDriver->origin.node->location;
		parentDriver->goal.node = person->destNode.node_;
		parentDriver->goal.point = parentDriver->goal.node->location;

		if(parentDriver->origin.node == parentDriver->goal.node){
			Print()
			<< "DriverMovement::initializePath | Can't initializePath(); origin and destination are the same for driver " <<person->GetId()
			<< "\norigin:" << parentDriver->origin.node->getID()
			<< "\ndestination:" << parentDriver->goal.node->getID()
			<< std::endl;
			return false;
		}

		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
		vector<WayPoint> wp_path = person->getCurrPath();
		if(wp_path.empty()){
			// if use path set
			if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
				throw std::runtime_error("todo: have not taken care of this profiling yet");
				sim_mob::Profiler profiler(true);
//				wp_path = PathSetManager::getInstance()->getPathByPerson(person);
				Worker* worker = (Worker*)(this->getParent()->currWorkerProvider);
				wp_path = worker->getPathSetMgr()->getPathByPerson(getParent());
				profiler.endProfiling();
			}
			else
			{
				const StreetDirectory& stdir = StreetDirectory::instance();
				wp_path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*(parentDriver->origin).node), stdir.DrivingVertex(*(parentDriver->goal).node));
			}
			person->setCurrPath(wp_path);
		}
		//For now, empty paths aren't supported.
		if (wp_path.empty()) {
			//throw std::runtime_error("Can't initializePath(); path is empty.");
			Print()<<"DriverMovement::initializePath | Can't initializePath(); path is empty for driver "
				   <<person->GetId()<<std::endl;
			return false;
		}
		std::vector<const sim_mob::SegmentStats*> path;
		initSegStatsPath(wp_path, path);
		if(path.empty()) {
			return false;
		}
		pathMover.setPath(path);
		const sim_mob::SegmentStats* firstSegStat = path.front();
		person->setCurrSegStats(firstSegStat);
		person->setCurrLane(firstSegStat->laneInfinity);
		person->distanceToEndOfSegment = firstSegStat->getLength();
	}
	//to indicate that the path to next activity is already planned
	person->setNextPathPlanned(true);
	return true;
}

void DriverMovement::setParentData(sim_mob::medium::DriverUpdateParams& params) {
	if(!pathMover.isPathCompleted()) {
		getParent()->distanceToEndOfSegment = pathMover.getPositionInSegment();
		getParent()->setCurrLane(currLane);
		getParent()->setCurrSegStats(pathMover.getCurrSegStats());
		getParent()->setRemainingTimeThisTick(params.secondsInTick - params.elapsedSeconds);
	}
	else {
		getParent()->distanceToEndOfSegment = 0.0;
		getParent()->setCurrLane(nullptr);
		getParent()->setCurrSegStats(nullptr);
		getParent()->setRemainingTimeThisTick(0.0);
		getParent()->isQueuing = false;
	}
}

void DriverMovement::stepFwdInTime(sim_mob::medium::DriverUpdateParams& params, double time) {
	params.elapsedSeconds = params.elapsedSeconds + time;
}

bool DriverMovement::advance(sim_mob::medium::DriverUpdateParams& params) {
	if (pathMover.isPathCompleted()) {
		getParent()->setToBeRemoved();
		return false;
	}

	if(getParent()->getRemainingTimeThisTick() <= 0){
		return false;
	}

	if (isQueuing)
	{
		return advanceQueuingVehicle(params);
	}
	else //vehicle is moving
	{
		return advanceMovingVehicle(params);
	}
}

bool DriverMovement::moveToNextSegment(sim_mob::medium::DriverUpdateParams& params) {
	bool res = false;
	bool isNewLinkNext = (!pathMover.hasNextSegStats(true) && pathMover.hasNextSegStats(false));
	const sim_mob::SegmentStats* currSegStat = pathMover.getCurrSegStats();
	const sim_mob::SegmentStats* nxtSegStat = pathMover.getNextSegStats(!isNewLinkNext);

	if (!nxtSegStat) {
		//vehicle is done
		pathMover.advanceInPath();
		if (pathMover.isPathCompleted()) {
			setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat)-1), currSegStat);
			currLane = nullptr;
			getParent()->setToBeRemoved();
		}
		return false;
	}

	if(isNewLinkNext) {
		getParent()->requestedNextSegStats = nxtSegStat;
		getParent()->canMoveToNextSegment = Person::NONE;
		return false; // return whenever a new link is to be entered. Seek permission from Conflux.
	}

	const sim_mob::SegmentStats* nextToNextSegStat = pathMover.getSecondSegStatsAhead();
	laneInNextSegment = getBestTargetLane(nxtSegStat, nextToNextSegStat);

	//this will space out the drivers on the same lane, by seperating them by the time taken for the previous car to move a car's length
	//Commenting out the delay from accept rate as per Yang Lu's suggestion (we only use this delay in setOrigin)
	double departTime = getLastAccept(laneInNextSegment, nxtSegStat)
			/* + getAcceptRate(laneInNextSegment, nxtSegStat)*/; //in seconds

	//skip acceptance capacity if there's no queue - this is done in DynaMIT
	//commenting out - the delay from acceptRate is removed as per Yang Lu's suggestion
/*	if(nextRdSeg->getParentConflux()->numQueueingInSegment(nextRdSeg, true) == 0){
		departTime = getLastAccept(nextLaneInNextSegment)
						+ (0.01 * vehicle->length) / (nextRdSeg->getParentConflux()->getSegmentSpeed(nextRdSeg, true) ); // skip input capacity
	}*/

	params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - converToSeconds(params.now.ms())); //in seconds

	if (canGoToNextRdSeg(params, nxtSegStat)){
		if (isQueuing){
			removeFromQueue();
		}

		setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat)-1), currSegStat); // decrement from the currLane before updating it
		currLane = laneInNextSegment;
		pathMover.advanceInPath();
		pathMover.setPositionInSegment(nxtSegStat->getLength());
		double segExitTimeSec =  params.elapsedSeconds + (converToSeconds(params.now.ms()));
		setLastAccept(currLane, segExitTimeSec, nxtSegStat);

		if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
			const sim_mob::SegmentStats* prevSegStats = pathMover.getPrevSegStats(true);	//previous segment is in the same link
			if(prevSegStats){
				// update road segment travel times
				updateRdSegTravelTimes(prevSegStats, segExitTimeSec);
			}
		}
		res = advance(params);
	}
	else {
		if (isQueuing){
			moveInQueue();
		}
		else{
			addToQueue();
		}
		params.elapsedSeconds = params.secondsInTick;
		getParent()->setRemainingTimeThisTick(0.0);
	}
	return res;
}

void DriverMovement::flowIntoNextLinkIfPossible(sim_mob::medium::DriverUpdateParams& params) {
	//This function gets called for 2 cases.
	//1. Driver is added to virtual queue
	//2. Driver is in previous segment trying to add to the next

	const sim_mob::SegmentStats* currSegStat = pathMover.getCurrSegStats();
	const sim_mob::SegmentStats* nextSegStats = pathMover.getNextSegStats(false);
	const sim_mob::SegmentStats* nextToNextSegStats = pathMover.getSecondSegStatsAhead();
	laneInNextSegment = getBestTargetLane(nextSegStats, nextToNextSegStats);

	//this will space out the drivers on the same lane, by seperating them by the time taken for the previous car to move a car's length
	//Commenting out the delay from accept rate as per Yang Lu's suggestion (we use this delay only in setOrigin)
	double departTime = getLastAccept(laneInNextSegment, nextSegStats)/* + getAcceptRate(laneInNextSegment, nextSegStats)*/; //in seconds

	params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (converToSeconds(params.now.ms()))); //in seconds

	if (canGoToNextRdSeg(params, nextSegStats)){
		if (isQueuing){
			removeFromQueue();
		}

		setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat)-1), currSegStat);
		currLane = laneInNextSegment;
		pathMover.advanceInPath();
		pathMover.setPositionInSegment(nextSegStats->getLength());

		double linkExitTimeSec =  params.elapsedSeconds + (converToSeconds(params.now.ms()));
		//set Link Travel time for previous link
		const SegmentStats* prevSegStats = pathMover.getPrevSegStats(false);
		if (prevSegStats) {
			// update link travel times
			updateLinkTravelTimes(prevSegStats, linkExitTimeSec);

			if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
				// update road segment travel times
				updateRdSegTravelTimes(prevSegStats, linkExitTimeSec);
			}
		}
		setLastAccept(currLane, linkExitTimeSec, nextSegStats);
		setParentData(params);
		getParent()->canMoveToNextSegment = Person::NONE;
	}
	else {
		//Person is in previous segment (should be added to queue if canGoTo failed)
		if(pathMover.getCurrSegStats() == getParent()->getCurrSegStats() ){
			if(currLane){
				if(getParent()->isQueuing) {
					moveInQueue();
				}
				else {
					addToQueue(); // adds to queue if not already in queue
				}
				getParent()->canMoveToNextSegment = Person::NONE; // so that advance() and setParentData() is called subsequently
			}
		}
		else if (pathMover.getNextSegStats(false) == getParent()->getCurrSegStats() ){
			//Person is in virtual queue (should remain in virtual queues if canGoTo failed)
			//do nothing
		}
		else{
			DebugStream << "Driver " << getParent()->getId()
					<< "was neither in virtual queue nor in previous segment!"
					<< "\ndriver| segment: " << pathMover.getCurrSegStats()->getRoadSegment()->getStartEnd()
					<< "|id: " << pathMover.getCurrSegStats()->getRoadSegment()->getSegmentID()
					<< "|lane: " << currLane->getLaneID()
					<< "\nPerson| segment: " << getParent()->getCurrSegStats()->getRoadSegment()->getStartEnd()
					<< "|id: " << getParent()->getCurrSegStats()->getRoadSegment()->getSegmentID()
					<< "|lane: " << (getParent()->getCurrLane()? getParent()->getCurrLane()->getLaneID():0)
					<< std::endl;

			throw::std::runtime_error(DebugStream.str());
		}
		params.elapsedSeconds = params.secondsInTick;
		getParent()->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
	}
}

bool DriverMovement::canGoToNextRdSeg(sim_mob::medium::DriverUpdateParams& params,
		const sim_mob::SegmentStats* nextSegStats) {
	//return false if the Driver cannot be added during this time tick
	if (params.elapsedSeconds >= params.secondsInTick) {
		return false;
	}

	//check if the next road segment has sufficient empty space to accommodate one more vehicle
	if (!nextSegStats) {
		return false;
	}

	double total = nextSegStats->getTotalVehicleLength();
	double max_allowed = nextSegStats->getNumVehicleLanes() * nextSegStats->getLength();
	return (total < max_allowed);
}

void DriverMovement::moveInQueue() {
	//1.update position in queue (vehicle->setPosition(distInQueue))
	//2.update p.timeThisTick
	double positionOfLastUpdatedAgentInLane =
			pathMover.getCurrSegStats()->getPositionOfLastUpdatedAgentInLane(currLane);

	if(positionOfLastUpdatedAgentInLane == -1.0) {
		pathMover.setPositionInSegment(0.0);
	}
	else {
		pathMover.setPositionInSegment(positionOfLastUpdatedAgentInLane
				+ parentDriver->getResource()->getLengthCm());
	}
}

bool DriverMovement::moveInSegment(double distance) {
	double startPos = pathMover.getPositionInSegment();

	try {
		pathMover.moveFwdInSegStats(distance);
	} catch (std::exception& ex) {
		std::stringstream msg;
		msg << "Error moving vehicle forward for Agent ID: " << getParent()->getId() << "," << pathMover.getPositionInSegment() << "\n" << ex.what();
		throw std::runtime_error(msg.str().c_str());
		return false;
	}

	double endPos = pathMover.getPositionInSegment();
	updateFlow(pathMover.getCurrSegStats(), startPos, endPos);

	return true;
}

bool DriverMovement::advanceQueuingVehicle(sim_mob::medium::DriverUpdateParams& params) {
	bool res = false;

	double initialTimeSpent = params.elapsedSeconds;
	double initialDistToSegEnd = pathMover.getPositionInSegment();
	double finalTimeSpent = 0.0;
	double finalDistToSegEnd = 0.0;

	double output = getOutputCounter(currLane, pathMover.getCurrSegStats());
	double outRate = getOutputFlowRate(currLane);

	//The following line of code assumes vehicle length is in cm;
	//vehicle length and outrate cannot be 0.
	//There was a magic factor 3.0 in the denominator. It was removed because
	//its purpose was not clear to anyone.~Harish
	finalTimeSpent = initialTimeSpent + initialDistToSegEnd/(PASSENGER_CAR_UNIT*outRate);

	if (output > 0 && finalTimeSpent < params.secondsInTick &&
			pathMover.getCurrSegStats()->getPositionOfLastUpdatedAgentInLane(currLane) == -1)
	{
		res = moveToNextSegment(params);
		finalDistToSegEnd = pathMover.getPositionInSegment();
	}
	else
	{
		moveInQueue();
		finalDistToSegEnd = pathMover.getPositionInSegment();
		params.elapsedSeconds =  params.secondsInTick;
	}
	//unless it is handled previously;
	//1. update current position of vehicle/driver with finalDistToSegEnd
	//2. update current time, p.elapsedSeconds, with finalTimeSpent
	pathMover.setPositionInSegment(finalDistToSegEnd);

	return res;
}

bool DriverMovement::advanceMovingVehicle(sim_mob::medium::DriverUpdateParams& params) {
	bool res = false;
	double initialTimeSpent = params.elapsedSeconds;
	double initialDistToSegEnd = pathMover.getPositionInSegment();
	double finalDistToSegEnd = 0.0;
	double finalTimeSpent = 0.0;

	if(!currLane) {
		throw std::runtime_error("agent's current lane is not set!");
	}

	const sim_mob::SegmentStats* currSegStats = pathMover.getCurrSegStats();
	//We can infer that the path is not completed if this function is called.
	//Therefore currSegStats cannot be NULL. It is safe to use it in this function.
	double velocity = currSegStats->getSegSpeed(true);
	double output = getOutputCounter(currLane, currSegStats);

	// add driver to queue if required
	double laneQueueLength = getQueueLength(currLane);
	if (laneQueueLength >  currSegStats->getLength())
	{
		addToQueue();
		params.elapsedSeconds = params.secondsInTick;
	}
	else if (laneQueueLength > 0)
	{
		finalTimeSpent = initialTimeSpent + (initialDistToSegEnd-laneQueueLength)/velocity; //time to reach end of queue

		if (finalTimeSpent < params.secondsInTick)
		{
			addToQueue();
			params.elapsedSeconds = params.secondsInTick;
		}
		else
		{
			finalDistToSegEnd = initialDistToSegEnd - (velocity * (params.secondsInTick - initialTimeSpent));
			res = moveInSegment(initialDistToSegEnd - finalDistToSegEnd);
			pathMover.setPositionInSegment(finalDistToSegEnd);
			params.elapsedSeconds = params.secondsInTick;
		}
	}
	else if (getInitialQueueLength(currLane) > 0)
	{
		res = advanceMovingVehicleWithInitialQ(params);
	}
	else //no queue or no initial queue
	{
		finalTimeSpent = initialTimeSpent + initialDistToSegEnd/velocity;
		if (finalTimeSpent < params.secondsInTick)
		{
			if (output > 0)
			{
				pathMover.setPositionInSegment(0.0);
				params.elapsedSeconds = finalTimeSpent;
				res = moveToNextSegment(params);
			}
			else
			{
				addToQueue();
				params.elapsedSeconds = params.secondsInTick;
			}
		}
		else
		{
			finalTimeSpent = params.secondsInTick;
			finalDistToSegEnd = initialDistToSegEnd-(velocity*(finalTimeSpent-initialTimeSpent));
			res = moveInSegment(initialDistToSegEnd-finalDistToSegEnd);
			pathMover.setPositionInSegment(finalDistToSegEnd);
			params.elapsedSeconds = finalTimeSpent;
		}
	}
	return res;
}

bool DriverMovement::advanceMovingVehicleWithInitialQ(sim_mob::medium::DriverUpdateParams& params) {
	bool res = false;
	double initialTimeSpent = params.elapsedSeconds;
	double initialDistToSegEnd = pathMover.getPositionInSegment();
	double finalTimeSpent = 0.0;
	double finalDistToSegEnd = 0.0;

	double velocity = pathMover.getCurrSegStats()->getSegSpeed(true);
	double output = getOutputCounter(currLane, pathMover.getCurrSegStats());
	double outRate = getOutputFlowRate(currLane);

	//The following line of code assumes vehicle length is in cm;
	//vehicle length and outrate cannot be 0.
	//There was a magic factor 3.0 in the denominator. It was removed because
	//its purpose was not clear to anyone. ~Harish
	double timeToDissipateQ = getInitialQueueLength(currLane)/(outRate*PASSENGER_CAR_UNIT);
	double timeToReachEndSeg = initialTimeSpent + initialDistToSegEnd/velocity;
	finalTimeSpent = std::max(timeToDissipateQ, timeToReachEndSeg);

	if (finalTimeSpent < params.secondsInTick)
	{
		if (output > 0)
		{
			pathMover.setPositionInSegment(0.0);
			params.elapsedSeconds = finalTimeSpent;
			res = moveToNextSegment(params);
		}
		else
		{
			addToQueue();
			params.elapsedSeconds = params.secondsInTick;
		}
	}
	else
	{
		if( fabs(finalTimeSpent-timeToReachEndSeg) < INFINITESIMAL_DOUBLE
				&& timeToReachEndSeg > params.secondsInTick)
		{
			finalTimeSpent = params.secondsInTick;
			finalDistToSegEnd = initialDistToSegEnd-velocity*(finalTimeSpent-initialTimeSpent);
			res = moveInSegment(initialDistToSegEnd - finalDistToSegEnd);
		}
		else
		{
			finalDistToSegEnd = 0.0 ;
			res = moveInSegment(initialDistToSegEnd - finalDistToSegEnd);
			finalTimeSpent = params.secondsInTick;
		}

		pathMover.setPositionInSegment(finalDistToSegEnd);
		params.elapsedSeconds = finalTimeSpent;
	}
	return res;
}

int DriverMovement::getOutputCounter(const Lane* lane, const sim_mob::SegmentStats* segStats) {
	return segStats->getLaneParams(lane)->getOutputCounter();
}

void DriverMovement::setOutputCounter(const Lane* lane, int count, const sim_mob::SegmentStats* segStats) {
	return segStats->getLaneParams(lane)->setOutputCounter(count);
}

double DriverMovement::getOutputFlowRate(const Lane* lane) {
	return pathMover.getCurrSegStats()->getLaneParams(lane)->getOutputFlowRate();
}

double DriverMovement::getAcceptRate(const Lane* lane, const sim_mob::SegmentStats* segStats) {
	return segStats->getLaneParams(lane)->getAcceptRate();
}

double DriverMovement::getQueueLength(const Lane* lane) {
	return pathMover.getCurrSegStats()->getLaneQueueLength(lane);
}

double DriverMovement::getLastAccept(const Lane* lane, const sim_mob::SegmentStats* segStats) {
	return segStats->getLaneParams(lane)->getLastAccept();
}

void DriverMovement::setLastAccept(const Lane* lane, double lastAccept, const sim_mob::SegmentStats* segStats) {
	segStats->getLaneParams(lane)->setLastAccept(lastAccept);
}

void DriverMovement::updateFlow(const sim_mob::SegmentStats* segStats, double startPos, double endPos) {
	double mid = segStats->getLength()/2.0;
	const sim_mob::RoadSegment* rdSeg = segStats->getRoadSegment();
	if (startPos >= mid && mid >= endPos){
		rdSeg->getParentConflux()->incrementSegmentFlow(rdSeg, segStats->getStatsNumberInSegment());
	}
}

void DriverMovement::setOrigin(sim_mob::medium::DriverUpdateParams& params) {
	if(params.now.ms() < getParent()->getStartTime()) {
		stepFwdInTime(params, (getParent()->getStartTime() - params.now.ms())/1000.0); //set time to start - to accommodate drivers starting during the frame
	}

	// here the person tries to move into a proper lane in the current segstats
	// from lane infinity
	const sim_mob::SegmentStats* currSegStats = pathMover.getCurrSegStats();
	const sim_mob::SegmentStats* nextSegStats = nullptr;
	if (pathMover.hasNextSegStats(true)) {
		nextSegStats = pathMover.getNextSegStats(true);
	}
	else if (pathMover.hasNextSegStats(false)) {
		nextSegStats = pathMover.getNextSegStats(false);
	}

	laneInNextSegment = getBestTargetLane(currSegStats, nextSegStats);

	//this will space out the drivers on the same lane, by seperating them by the time taken for the previous car to move a car's length
	double departTime = getLastAccept(laneInNextSegment, currSegStats) + getAcceptRate(laneInNextSegment, currSegStats); //in seconds

	/*//skip acceptance capacity if there's no queue - this is done in DynaMIT
	if(getCurrSegment()->getParentConflux()->numQueueingInSegment(getCurrSegment(), true) == 0){
		departTime = getLastAccept(nextLaneInNextSegment)
						+ (0.01 * vehicle->length) / (getCurrSegment()->getParentConflux()->getSegmentSpeed(getCurrSegment(), true) ); // skip input capacity
	}*/

	params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (converToSeconds(params.now.ms())));	//in seconds

	if(canGoToNextRdSeg(params, currSegStats))
	{
		//set position to start
		if(currSegStats)
		{
			pathMover.setPositionInSegment(currSegStats->getLength());
		}
		currLane = laneInNextSegment;
		double actualT = params.elapsedSeconds + (converToSeconds(params.now.ms()));
		getParent()->initLinkTravelStats(currSegStats->getRoadSegment()->getLink(), actualT);

		setLastAccept(currLane, actualT, currSegStats);
		setParentData(params);
		getParent()->canMoveToNextSegment = Person::NONE;
	}
	else
	{
		params.elapsedSeconds = params.secondsInTick;
		getParent()->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
	}
}

bool DriverMovement::isConnectedToNextSeg(const Lane* lane, const SegmentStats* nxtSegStat) {
	if(!nxtSegStat) {
		throw std::runtime_error("DriverMovement::isConnectedToNextSeg() - nxtSegStat is not available!");
	}

	const sim_mob::RoadSegment* nxtRdSeg = nxtSegStat->getRoadSegment();
	if (nxtRdSeg->getLink() != lane->getRoadSegment()->getLink()){
		const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (lane->getRoadSegment()->getEnd());
		if (currEndNode) {
			const set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(lane->getRoadSegment());
			for (set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++) {
				if ((*it)->getLaneTo()->getRoadSegment() == nxtRdSeg && (*it)->getLaneFrom() == lane) {
					return true;
				}
			}
		}
	}
	else{
		//if (lane->getRoadSegment()->getLink() == nxtRdSeg->getLink()) we are
		//crossing a uni-node. At uninodes, we assume all lanes of the current
		//segment are connected to all lanes of the next segment
		return true;
	}
	return false;
}

void DriverMovement::addToQueue() {
	/* 1. set position to queue length in front
	 * 2. set isQueuing = true
	*/
	Person* parentP = getParent();
	if (parentP) {
		if(!parentP->isQueuing) {
			pathMover.setPositionInSegment(getQueueLength(currLane));
			isQueuing = true;
			parentP->isQueuing = isQueuing;
		}
		else {
			DebugStream << "addToQueue() was called for a driver who is already in queue. Person: " << parentP->getId()
					<< "|RoadSegment: " << currLane->getRoadSegment()->getStartEnd()
					<< "|Lane: " << currLane->getLaneID() << std::endl;
			throw std::runtime_error(DebugStream.str());
		}
	}
}

void DriverMovement::removeFromQueue() {
	Person* parentP = getParent();
	if (parentP) {
		if(parentP->isQueuing) {
			parentP->isQueuing = false;
			isQueuing = false;
		}
		else {
			 DebugStream << "removeFromQueue() was called for a driver who is not in queue. Person: " << parentP->getId()
					<< "|RoadSegment: " << currLane->getRoadSegment()->getStartEnd()
					<< "|Lane: " << currLane->getLaneID() << std::endl;
			 throw std::runtime_error(DebugStream.str());
		}
	}
}

const sim_mob::Lane* DriverMovement::getBestTargetLane(
		const SegmentStats* nextSegStats,
		const SegmentStats* nextToNextSegStats)
{
	if(!nextSegStats) { return nullptr; }
	const sim_mob::Lane* minLane = nullptr;
	double minQueueLength = std::numeric_limits<double>::max();
	double minLength = std::numeric_limits<double>::max();
	double que = 0.0;
	double total = 0.0;

	const std::vector<sim_mob::Lane*>& lanes = nextSegStats->getRoadSegment()->getLanes();
	for (vector<sim_mob::Lane* >::const_iterator lnIt = lanes.begin(); lnIt != lanes.end(); ++lnIt)
	{
		const Lane* lane = *lnIt;
		if (!lane->is_pedestrian_lane() && !lane->is_whole_day_bus_lane())
		{
			if(nextToNextSegStats && !isConnectedToNextSeg(lane, nextToNextSegStats)) {	continue; }
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

	if(!minLane) { throw std::runtime_error("best target lane was not set!"); }
	return minLane;
}

double DriverMovement::getInitialQueueLength(const Lane* lane) {
	return getParent()->getCurrSegStats()->getInitialQueueLength(lane);
}

void DriverMovement::updateLinkTravelTimes(const sim_mob::SegmentStats* prevSegStat, double linkExitTimeSec){
	const RoadSegment* prevSeg= prevSegStat->getRoadSegment();
	const Link* prevLink = prevSeg->getLink();
	if(prevLink == getParent()->getLinkTravelStats().link_){
		getParent()->addToLinkTravelStatsMap(getParent()->getLinkTravelStats(), linkExitTimeSec); //in seconds
		prevSegStat->getRoadSegment()->getParentConflux()->setLinkTravelTimes(getParent(), linkExitTimeSec);
	}
	//creating a new entry in agent's travelStats for the new link, with entry time
	getParent()->initLinkTravelStats(pathMover.getCurrSegStats()->getRoadSegment()->getLink(), linkExitTimeSec);
}

void DriverMovement::updateRdSegTravelTimes(const sim_mob::SegmentStats* prevSegStat, double segStatExitTimeSec){
	//if prevSeg is already in travelStats, update it's rdSegTT and add to rdSegTravelStatsMap
	const RoadSegment* prevSeg= prevSegStat->getRoadSegment();
	if(prevSeg == getParent()->getRdSegTravelStats().rdSeg_){
		getParent()->addToRdSegTravelStatsMap(getParent()->getRdSegTravelStats(), segStatExitTimeSec); //in seconds
		prevSeg->getParentConflux()->setRdSegTravelTimes(getParent(), segStatExitTimeSec);
	}
	//creating a new entry in agent's travelStats for the new road segment, with entry time
	getParent()->initRdSegTravelStats(pathMover.getCurrSegStats()->getRoadSegment(), segStatExitTimeSec);
}

int DriverMovement::findReroutingPoints(const std::vector<sim_mob::SegmentStats*>& stats, std::vector<const sim_mob::Node*> & intersections, centimeter_t & distance) const{
	const std::vector<const sim_mob::SegmentStats*> & path = getMesoPathMover().getPath();

	//find an iterator to the incident point
	std::vector<const sim_mob::SegmentStats*>::const_iterator endIt = std::find(path.begin(), path.end(), *(stats.begin()));
	if(endIt == path.end()){
		throw std::runtime_error("Incidnt is not on this path!");
	}

	//find an iterator to the beginning point of your search to the incident point
	std::vector<const sim_mob::SegmentStats*>::const_iterator startIt = std::find(path.begin(), path.end(), getMesoPathMover().getCurrSegStats());

	//count the number of links as you iterate/traverse through the path. the count will be the number of intersections on your way.
	const sim_mob::Link * currLink = (*startIt)->getRoadSegment()->getLink();
	int res = 0;
	for(startIt++ ; startIt <= endIt; startIt++)
	{
		if(currLink != (*startIt)->getRoadSegment()->getLink()){
			//new link->new intersection->new point of re-routing
			intersections.push_back(currLink->getEnd());
			++res;
			currLink = (*startIt)->getRoadSegment()->getLink();
		}
	}
	return res;
}
void DriverMovement::rerout(const InsertIncidentMessage &msg){
//	PathSetManager::getInstance()->getPathByPerson()
	//step-1 can I rerout? if yes, what are my points of rerout?
		//criterion-1 at least 1 intersection from where the agent is, to the troubled roadsegment
	std::vector<const sim_mob::Node*> intersections = std::vector<const sim_mob::Node*>();
	centimeter_t distance = 0;
	int numReRoute = findReroutingPoints(msg.stats,intersections, distance);
	if(!numReRoute){
		return;
	}
	//step-2 do I 'want' to reroute? yes you do, for now
	if(!wantReRoute()){
		return;
	}

	//step-3:get a new path

}

void DriverMovement::HandleMessage(messaging::Message::MessageType type,
		const messaging::Message& message){
	throw std::runtime_error("Reaching the medium::DriverMovement::HandleMessage");
	switch (type){
	case MSG_INSERT_INCIDENT:{
		const InsertIncidentMessage &msg = MSG_CAST(InsertIncidentMessage,message);
		rerout(msg);
		break;
	}
	}
}

} /* namespace medium */
} /* namespace sim_mob */
