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
#include "entities/misc/TripChain.hpp"
#include "entities/conflux/Conflux.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
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

using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;

namespace {
void initSegStatsPath(vector<sim_mob::WayPoint>& wpPath,
		vector<const sim_mob::SegmentStats*>& ssPath) {
	for (vector<sim_mob::WayPoint>::iterator it = wpPath.begin();
			it != wpPath.end(); it++) {
		if (it->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* rdSeg = it->roadSegment_;
			const sim_mob::SegmentStats* segStats =
					rdSeg->getParentConflux()->findSegStats(rdSeg);
			ssPath.push_back(segStats);
		}
	}
}
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


sim_mob::medium::DriverMovement::DriverMovement(sim_mob::Person* parentAgent):
	MovementFacet(parentAgent), parentDriver(nullptr), vehicleLength(400),
	currLane(nullptr), laneInNextSegment(nullptr), isQueuing(false),
	velocity(0)
{}

sim_mob::medium::DriverMovement::~DriverMovement() {}

void sim_mob::medium::DriverMovement::frame_init() {
	bool pathInitialized = initializePath();
	if (pathInitialized) {
		Vehicle* newVeh = new Vehicle();
		Vehicle* oldVehicle = parentDriver->getResource();
		safe_delete_item(oldVehicle);
		parentDriver->setResource(newVeh);
	}
	else{
		getParent()->setToBeRemoved();
	}
}

void sim_mob::medium::DriverMovement::frame_tick() {
	sim_mob::medium::DriverUpdateParams& p2 = parentDriver->getParams();
	const sim_mob::SegmentStats* currSegStats = pathMover.getCurrSegStats();
	if(currSegStats == getParent()->getCurrSegStats())
	{
		if (!pathMover.isPathCompleted() && currSegStats->laneInfinity)
		{
			//the vehicle will be in lane infinity before it starts starts. set origin will move it to the correct lane
			if (getParent()->getCurrLane() == currSegStats->laneInfinity){
				setOrigin(p2);
			}
		} else {
			Warn() <<"ERROR: Vehicle could not be created for driver; no route!" <<std::endl;
		}
	}

	//Are we done already?
	if (pathMover.isPathCompleted()) {
		getParent()->setToBeRemoved();
		return;
	}

	//======================================incident==========================================
/*
	// this needs to be moved to be changed to read from input xml later
	const sim_mob::RoadSegment* nextRdSeg = nullptr;
	if (hasNextSegment(true))
		nextRdSeg = getNextSegment(true);

	else if (hasNextSegment(false))
		nextRdSeg = getNextSegment(false);

	if (nextRdSeg){
		if(nextRdSeg->getStart()->getID() == 84882){
		std::cout << "adding incident "<<p.now.ms() << " "<< parent->getId()
				<<" outputFlowRate: "<<getOutputFlowRate(parent->getCurrLane())<<std::endl;
		if (getOutputFlowRate(nextRdSeg->getLanes()[0]) != 0 &&
				nextRdSeg->getStart()->getID() == 84882 && p.now.ms() == 6000){
			std::cout << "incident added." << p.now.ms() << std::endl;
			insertIncident(nextRdSeg, 0);
		}
		}
	}

//	if (getCurrSegment()->getStart()->getID() == 103046 && p.now.ms() == 21000
	//		and parent->getId() == 46){
		//	std::cout << "incident removed." << std::endl;
			//removeIncident(getCurrSegment());
	//}
*/
	//=====================================incident==============================================
	if(getParent()->canMoveToNextSegment == Person::GRANTED) {
		flowIntoNextLinkIfPossible(p2);
	}
	else if (getParent()->canMoveToNextSegment == Person::DENIED){
		if(currLane) {
			if(getParent()->isQueuing) {
				moveInQueue();
			}
			else {
				addToQueue(currLane); // adds to queue if not already in queue
			}

			p2.elapsedSeconds = p2.secondsInTick;
			getParent()->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
			setParentData(p2);
		}
	}
	//if vehicle is still in lane infinity, it shouldn't be advanced
	if (currLane && getParent()->canMoveToNextSegment == Person::NONE) {
		advance(p2);
		setParentData(p2);
	}
}

void sim_mob::medium::DriverMovement::frame_tick_output() {
	const sim_mob::medium::DriverUpdateParams& p = parentDriver->getParams();
	if (pathMover.isPathCompleted() || ConfigManager::GetInstance().FullConfig().using_MPI || ConfigManager::GetInstance().CMakeConfig().OutputDisabled()) {
		return;
	}

	std::stringstream logout;
	logout << "(\"Driver\""
			<<","<<getParent()->getId()
			<<","<<p.now.frame()
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
				wp_path = PathSetManager::getInstance()->getPathByPerson(person);
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

void DriverMovement::setParentData(sim_mob::medium::DriverUpdateParams& p) {
	if(!pathMover.isPathCompleted()) {
		getParent()->distanceToEndOfSegment = pathMover.getPositionInSegment();
		getParent()->setCurrLane(currLane);
		getParent()->setCurrSegStats(pathMover.getCurrSegStats());
		getParent()->setRemainingTimeThisTick(p.secondsInTick - p.elapsedSeconds);
	}
	else {
		getParent()->distanceToEndOfSegment = 0.0;
		getParent()->setCurrLane(nullptr);
		getParent()->setCurrSegStats(nullptr);
		getParent()->setRemainingTimeThisTick(0.0);
		getParent()->isQueuing = false;
	}
}

void DriverMovement::stepFwdInTime(sim_mob::medium::DriverUpdateParams& p, double time) {
	p.elapsedSeconds = p.elapsedSeconds + time;
}

bool DriverMovement::advance(sim_mob::medium::DriverUpdateParams& p) {
	if (pathMover.isPathCompleted()) {
		getParent()->setToBeRemoved();
		return false;
	}

	if(getParent()->getRemainingTimeThisTick() <= 0){
		return false;
	}

	if (isQueuing)
	{
		return advanceQueuingVehicle(p);
	}
	else //vehicle is moving
	{
		return advanceMovingVehicle(p);
	}
}

bool DriverMovement::moveToNextSegment(sim_mob::medium::DriverUpdateParams& p) {
	bool res = false;
	bool isNewLinkNext = (!pathMover.hasNextSegStats(true) && pathMover.hasNextSegStats(false));
	const sim_mob::SegmentStats* currSegStat = pathMover.getCurrSegStats();
	const sim_mob::SegmentStats* nxtSegStat = nullptr;

	if (isNewLinkNext) {
		nxtSegStat = pathMover.getNextSegStats(false);
	}
	else {
		nxtSegStat = pathMover.getNextSegStats(true);
	}

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

	p.elapsedSeconds = std::max(p.elapsedSeconds, departTime - (p.now.ms()/1000.0)); //in seconds

	if (canGoToNextRdSeg(p, nxtSegStat)){
		if (isQueuing){
			removeFromQueue();
		}

		setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat)-1), currSegStat); // decrement from the currLane before updating it
		currLane = laneInNextSegment;
		pathMover.advanceInPath();
		pathMover.setPositionInSegment(nxtSegStat->getLength());
		double linkExitTimeSec =  p.elapsedSeconds + (p.now.ms()/1000.0);
		setLastAccept(currLane, linkExitTimeSec, nxtSegStat);

		if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
			const sim_mob::SegmentStats* prevSegStats = pathMover.getPrevSegStats(true);	//previous segment is in the same link
			if(prevSegStats){
				// update road segment travel times
				updateRdSegTravelTimes(prevSegStats, linkExitTimeSec);
			}
		}
		res = advance(p);
	}
	else{
		if (isQueuing){
			moveInQueue();
		}
		else{
			addToQueue(currLane);
		}
		p.elapsedSeconds = p.secondsInTick;
		getParent()->setRemainingTimeThisTick(0.0);
	}
	return res;
}

void DriverMovement::flowIntoNextLinkIfPossible(UpdateParams& up) {
	//This function gets called for 2 cases.
	//1. Driver is added to virtual queue
	//2. Driver is in previous segment trying to add to the next

	sim_mob::medium::DriverUpdateParams& p = dynamic_cast<sim_mob::medium::DriverUpdateParams&>(up);
	const sim_mob::SegmentStats* currSegStat = pathMover.getCurrSegStats();
	const sim_mob::SegmentStats* nextSegStats = pathMover.getNextSegStats(false);
	const sim_mob::SegmentStats* nextToNextSegStats = pathMover.getSecondSegStatsAhead();
	laneInNextSegment = getBestTargetLane(nextSegStats, nextToNextSegStats);

	//this will space out the drivers on the same lane, by seperating them by the time taken for the previous car to move a car's length
	//Commenting out the delay from accept rate as per Yang Lu's suggestion (we use this delay only in setOrigin)
	double departTime = getLastAccept(laneInNextSegment, nextSegStats)/* + getAcceptRate(laneInNextSegment, nextSegStats)*/; //in seconds

	p.elapsedSeconds = std::max(p.elapsedSeconds, departTime - (p.now.ms()/1000.0)); //in seconds

	if (canGoToNextRdSeg(p, nextSegStats)){
		if (isQueuing){
			removeFromQueue();
		}

		setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat)-1), currSegStat);
		currLane = laneInNextSegment;
		pathMover.advanceInPath();
		pathMover.setPositionInSegment(nextSegStats->getLength());

		double linkExitTimeSec =  p.elapsedSeconds + (p.now.ms()/1000.0);
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
		setParentData(p);
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
					addToQueue(currLane); // adds to queue if not already in queue
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
		p.elapsedSeconds = p.secondsInTick;
		getParent()->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
	}
}

bool DriverMovement::canGoToNextRdSeg(sim_mob::medium::DriverUpdateParams& p,
		const sim_mob::SegmentStats* nextSegStats) {
	//return false if the Driver cannot be added during this time tick
	if (p.elapsedSeconds >= p.secondsInTick) {
		return false;
	}

	//check if the next road segment has sufficient empty space to accommodate one more vehicle
	if (!nextSegStats) {
		return false;
	}

	unsigned int total = nextSegStats->numMovingInSegment(true)
						+ nextSegStats->numQueueingInSegment(true);

	int vehLaneCount = nextSegStats->getNumVehicleLanes();
	double max_allowed = (vehLaneCount * nextSegStats->getLength()/vehicleLength);
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
		pathMover.setPositionInSegment(positionOfLastUpdatedAgentInLane +  vehicleLength);
	}
}

bool DriverMovement::moveInSegment(sim_mob::medium::DriverUpdateParams& p2, double distance) {
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
	updateFlow(pathMover.getCurrSegStats()->getRoadSegment(), startPos, endPos);

	return true;
}

bool DriverMovement::advanceQueuingVehicle(sim_mob::medium::DriverUpdateParams& p) {
	bool res = false;

	double t0 = p.elapsedSeconds;
	double x0 = pathMover.getPositionInSegment();
	double xf = 0.0;
	double tf = 0.0;

	double output = getOutputCounter(currLane, pathMover.getCurrSegStats());
	double outRate = getOutputFlowRate(currLane);
	tf = t0 + x0/(3.0*vehicleLength*outRate); //assuming vehicle length is in cm
	if (output > 0 && tf < p.secondsInTick &&
			currLane->getRoadSegment()->getParentConflux()->getPositionOfLastUpdatedAgentInLane(currLane) == -1)
	{
		res = moveToNextSegment(p);
		xf = pathMover.getPositionInSegment();
	}
	else
	{
		moveInQueue();
		xf = pathMover.getPositionInSegment();
		p.elapsedSeconds =  p.secondsInTick;
	}
	//unless it is handled previously;
	//1. update current position of vehicle/driver with xf
	//2. update current time, p.timeThisTick, with tf
	pathMover.setPositionInSegment(xf);

	return res;
}

bool DriverMovement::advanceMovingVehicle(sim_mob::medium::DriverUpdateParams& p) {

	bool res = false;
	double t0 = p.elapsedSeconds;
	double x0 = pathMover.getPositionInSegment();
//	std::cout<<"rdSeg: "<<pathMover.getPositionInSegment()<<std::endl;
	double xf = 0.0;
	double tf = 0.0;

	if(!currLane) {
		throw std::runtime_error("agent's current lane is not set!");
	}

	getSegSpeed();

	double vu = velocity;

	double output = getOutputCounter(currLane, pathMover.getCurrSegStats());

	//get current location
	//before checking if the vehicle should be added to a queue, it's re-assigned to the best lane
	double laneQueueLength = getQueueLength(currLane);
	//if (laneQueueLength > vehicle->getCurrLinkLaneZeroLength() )
	if (laneQueueLength >  currLane->getRoadSegment()->getLaneZeroLength())
	{
		addToQueue(currLane);
		p.elapsedSeconds = p.secondsInTick;
	}
	else if (laneQueueLength > 0)
	{
		tf = t0 + (x0-laneQueueLength)/vu; //time to reach end of queue

		if (tf < p.secondsInTick)
		{
			addToQueue(currLane);
			p.elapsedSeconds = p.secondsInTick;
		}
		else
		{
			xf = x0 - vu * (p.secondsInTick - t0);
			res = moveInSegment(p, x0 - xf);
			pathMover.setPositionInSegment(xf);
			p.elapsedSeconds = p.secondsInTick;
		}
	}
	else if (getInitialQueueLength(currLane) > 0)
	{
		res = advanceMovingVehicleWithInitialQ(p);
	}
	else //no queue or no initial queue
	{
		tf = t0 + x0/vu;
		if (tf < p.secondsInTick)
		{
			if (output > 0)
			{
				pathMover.setPositionInSegment(0.0);
				p.elapsedSeconds = tf;
				res = moveToNextSegment(p);
			}
			else
			{
				addToQueue(currLane);
				p.elapsedSeconds = p.secondsInTick;
			}
		}
		else
		{
			tf = p.secondsInTick;
			xf = x0-vu*(tf-t0);
			res = moveInSegment(p, x0-xf);
			pathMover.setPositionInSegment(xf);
			p.elapsedSeconds = tf;
		}
	}
	return res;
}

bool DriverMovement::advanceMovingVehicleWithInitialQ(sim_mob::medium::DriverUpdateParams& p) {

	bool res = false;
	double t0 = p.elapsedSeconds;
	double x0 = pathMover.getPositionInSegment();
	double xf = 0.0;
	double tf = 0.0;

	getSegSpeed();
	double vu = velocity;

	double output = getOutputCounter(currLane, pathMover.getCurrSegStats());
	double outRate = getOutputFlowRate(currLane);

	double timeToDissipateQ = getInitialQueueLength(currLane)/(3.0*outRate*vehicleLength); //assuming vehicle length is in cm
	double timeToReachEndSeg = t0 + x0/vu;
	tf = std::max(timeToDissipateQ, timeToReachEndSeg);

	if (tf < p.secondsInTick)
	{
		if (output > 0)
		{
			pathMover.setPositionInSegment(0.0);
			p.elapsedSeconds = tf;
			res = moveToNextSegment(p);
		}
		else
		{
			addToQueue(currLane);
			p.elapsedSeconds = p.secondsInTick;
		}
	}
	else
	{
		if( fabs(tf-timeToReachEndSeg) < 0.001 && timeToReachEndSeg > p.secondsInTick)
		{
			tf = p.secondsInTick;
			xf = x0-vu*(tf-t0);
			res = moveInSegment(p, x0-xf);
		}
		else
		{
			xf = 0.0 ;
			res = moveInSegment(p, x0-xf);
			tf = p.secondsInTick;
		}

		pathMover.setPositionInSegment(xf);
		p.elapsedSeconds = tf;
	}
	return res;
}

void DriverMovement::getSegSpeed() {
	velocity = pathMover.getCurrSegStats()->getSegSpeed(true);
}

int DriverMovement::getOutputCounter(const Lane* l, const sim_mob::SegmentStats* segStats) {
	return segStats->getLaneParams(l)->getOutputCounter();
}

void DriverMovement::setOutputCounter(const Lane* l, int count, const sim_mob::SegmentStats* segStats) {
	return segStats->getLaneParams(l)->setOutputCounter(count);
}

double DriverMovement::getOutputFlowRate(const Lane* l) {
	return pathMover.getCurrSegStats()->getLaneParams(l)->getOutputFlowRate();
}

double DriverMovement::getAcceptRate(const Lane* l, const sim_mob::SegmentStats* segStats) {
	return segStats->getLaneParams(l)->getAcceptRate();
}

double DriverMovement::getQueueLength(const Lane* l) {
	return ((pathMover.getCurrSegStats()->getLaneAgentCounts(l)).first) * (vehicleLength);
}

double DriverMovement::getLastAccept(const Lane* l, const sim_mob::SegmentStats* segStats) {
	return segStats->getLaneParams(l)->getLastAccept();
}

void DriverMovement::setLastAccept(const Lane* l, double lastAccept, const sim_mob::SegmentStats* segStats) {
	segStats->getLaneParams(l)->setLastAccept(lastAccept);
}

void DriverMovement::updateFlow(const sim_mob::RoadSegment* rdSeg, double startPos, double endPos) {
	double mid = rdSeg->getLaneZeroLength()/2.0;
	if (startPos >= mid && mid >= endPos){
		rdSeg->getParentConflux()->incrementSegmentFlow(rdSeg);
	}
}

void DriverMovement::setOrigin(sim_mob::medium::DriverUpdateParams& p) {

	//Vehicles start at rest
	velocity = 0;

	if(p.now.ms() < getParent()->getStartTime()) {
		stepFwdInTime(p, (getParent()->getStartTime() - p.now.ms())/1000.0); //set time to start - to accommodate drivers starting during the frame
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

	p.elapsedSeconds = std::max(p.elapsedSeconds, departTime - (p.now.ms()/1000.0));	//in seconds

	if(canGoToNextRdSeg(p, currSegStats))
	{
		//set position to start
		if(currSegStats)
		{
			pathMover.setPositionInSegment(currSegStats->getLength());
		}
		currLane = laneInNextSegment;
		double actualT = p.elapsedSeconds + (p.now.ms()/1000.0);
		getParent()->initLinkTravelStats(currSegStats->getRoadSegment()->getLink(), actualT);

		setLastAccept(currLane, actualT, currSegStats);
		setParentData(p);
		getParent()->canMoveToNextSegment = Person::NONE;
	}
	else
	{
		p.elapsedSeconds = p.secondsInTick;
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
		// if (lane->getRoadSegment()->getLink() == nxtRdSeg->getLink())
		//handling uni-nodes - where lanes should be connected to the same outgoing segment
		// at uninodes, we assume all lanes of the current segment are connected to all lanes of the next segment
		return true;
	}
	return false;
}

void DriverMovement::addToQueue(const Lane* lane) {
	/* 1. set position to queue length in front
	 * 2. set isQueuing = true
	*/
	Person* parentP = getParent();
	if (parentP) {
		if(!parentP->isQueuing) {
			pathMover.setPositionInSegment(getQueueLength(lane));
			isQueuing = true;
			parentP->isQueuing = isQueuing;
		}
		else {
			DebugStream << "addToQueue() was called for a driver who is already in queue. Person: " << parentP->getId()
					<< "|RoadSegment: " << lane->getRoadSegment()->getStartEnd()
					<< "|Lane: " << lane->getLaneID() << std::endl;
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

const sim_mob::Lane* DriverMovement::getBestTargetLane(const SegmentStats* nextSegStats, const SegmentStats* nextToNextSegStats) {
	//we have included getBastLG functionality here (get lane with minAllAgents)
	//before checking best lane
	//1. Get queuing counts for all lanes of the next Segment
	//2. Select the lane with the least queue length
	//3. Update nextLaneInNextLink and targetLaneIndex accordingly
	if(!nextSegStats) {
		return nullptr;
	}

	const sim_mob::Lane* minQueueLengthLane = nullptr;
	unsigned int minQueueLength = std::numeric_limits<int>::max();
	unsigned int minAllAgents = std::numeric_limits<int>::max();
	unsigned int que = 0;
	unsigned int total = 0;
	int test_count = 0;

	const std::vector<sim_mob::Lane*>& lanes = nextSegStats->getRoadSegment()->getLanes();
	vector<sim_mob::Lane* >::const_iterator lnIt = lanes.begin();
	vector<sim_mob::Lane* > laneGroup;	//temporary container to save the connected lanes

	//getBestLaneGroup logic
	for (;lnIt != lanes.end(); ++lnIt){
		if (!((*lnIt)->is_pedestrian_lane())){
			if(nextToNextSegStats && !isConnectedToNextSeg(*lnIt, nextToNextSegStats))	{
				continue;
			}
			laneGroup.push_back(*lnIt);
			std::pair<unsigned int, unsigned int> counts = nextSegStats->getLaneAgentCounts(*lnIt); //<Q,M>
			total = counts.first + counts.second;

			if (minAllAgents > total){
				minAllAgents = total;
			}
		}
	}

	//getBestLane logic
	for (lnIt = laneGroup.begin(); lnIt != laneGroup.end(); lnIt++){
		std::pair<unsigned int, unsigned int> counts = nextSegStats->getLaneAgentCounts(*lnIt); //<Q, M>
		que = counts.first;
		total = que + counts.second;
		if (minAllAgents == total){
			if (minQueueLength > que){
				minQueueLength = que;
				minQueueLengthLane = *lnIt;
			}
		}
	}

	laneGroup.clear();

	if(!minQueueLengthLane){
		throw std::runtime_error("best target lane was not set!");
	}
	return minQueueLengthLane;
}

double DriverMovement::getInitialQueueLength(const Lane* l) {
	return getParent()->getCurrSegStats()->getInitialQueueCount(l) * vehicleLength;
}

void DriverMovement::insertIncident(const RoadSegment* rdSeg, double newFlowRate) {
	const vector<Lane*> lanes = rdSeg->getLanes();
	for (vector<Lane*>::const_iterator it = lanes.begin(); it != lanes.end(); it++) {
		rdSeg->getParentConflux()->updateLaneParams((*it), newFlowRate);
	}
}

void DriverMovement::removeIncident(const RoadSegment* rdSeg) {
	const vector<Lane*> lanes = rdSeg->getLanes();
	for (vector<Lane*>::const_iterator it = lanes.begin(); it != lanes.end(); it++){
		rdSeg->getParentConflux()->restoreLaneParams(*it);
	}
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

} /* namespace medium */
} /* namespace sim_mob */
