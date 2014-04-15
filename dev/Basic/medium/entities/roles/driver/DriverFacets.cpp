//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverFacets.hpp"
#include <cmath>
#include <ostream>
#include <algorithm>

#include "buffering/BufferedDataManager.hpp"

#include "entities/Person.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/conflux/Conflux.hpp"

#include "buffering/BufferedDataManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
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
	MovementFacet(parentAgent), parentDriver(nullptr), vehicle(nullptr), currLane(nullptr), nextLaneInNextSegment(nullptr) {}

sim_mob::medium::DriverMovement::~DriverMovement() {
	safe_delete_item(vehicle);
}

void sim_mob::medium::DriverMovement::frame_init() {
	//Save the path from orign to next activity location in allRoadSegments
	Vehicle* newVeh = initializePath(true);
	if (newVeh) {
		safe_delete_item(vehicle);
		vehicle = newVeh;
		parentDriver->setResource(newVeh);
	}
	else{
		getParent()->setToBeRemoved();
	}
}

void sim_mob::medium::DriverMovement::frame_tick() {
	DriverUpdateParams& p2 = parentDriver->getParams();
	const Lane* laneInfinity = vehicle->getCurrSegment()->getParentConflux()->getLaneInfinity(vehicle->getCurrSegment());
	if(vehicle->getCurrSegment() == getParent()->getCurrSegment() )
	{
		if (vehicle->hasPath() && laneInfinity)
		{
			//the vehicle will be in lane infinity before it starts starts. set origin will move it to the correct lane
			if (getParent()->getCurrLane() == laneInfinity){
				setOrigin(p2);
			}
		} else {
			Warn() <<"ERROR: Vehicle could not be created for driver; no route!" <<std::endl;
		}
	}

	//Are we done already?
	if (vehicle->isDone()) {
		getParent()->setToBeRemoved();
		return;
	}

	//======================================incident==========================================
/*
	// this needs to be moved to be changed to read from input xml later
	const sim_mob::RoadSegment* nextRdSeg = nullptr;
	if (vehicle->hasNextSegment(true))
		nextRdSeg = vehicle->getNextSegment(true);

	else if (vehicle->hasNextSegment(false))
		nextRdSeg = vehicle->getNextSegment(false);

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

//	if (vehicle->getCurrSegment()->getStart()->getID() == 103046 && p.now.ms() == 21000
	//		and parent->getId() == 46){
		//	std::cout << "incident removed." << std::endl;
			//removeIncident(vehicle->getCurrSegment());
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
	const DriverUpdateParams& p = parentDriver->getParams();
	if (vehicle->isDone() || ConfigManager::GetInstance().FullConfig().using_MPI || ConfigManager::GetInstance().CMakeConfig().OutputDisabled()) {
		return;
	}

	std::stringstream logout;
	logout << "(\"Driver\""
			<<","<<getParent()->getId()
			<<","<<p.now.frame()
			<<",{"
			<<"\"RoadSegment\":\""<< (getParent()->getCurrSegment()->getSegmentID())
			<<"\",\"Lane\":\""<<((getParent()->getCurrLane())? getParent()->getCurrLane()->getLaneID():0)
			<<"\",\"Segment\":\""<<(getParent()->getCurrSegment()->getStartEnd())
			<<"\",\"DistanceToEndSeg\":\""<<getParent()->distanceToEndOfSegment;
	if (this->getParent()->isQueuing) {
			logout << "\",\"queuing\":\"" << "true";
	} else {
			logout << "\",\"queuing\":\"" << "false";
	}
	logout << "\"})" << std::endl;

	LogOut(logout.str());
}

sim_mob::Vehicle* sim_mob::medium::DriverMovement::initializePath(bool allocateVehicle) {
	Vehicle* res = nullptr;

	//Only initialize if the next path has not been planned for yet.
	if(!getParent()->getNextPathPlanned()){
		//Save local copies of the parent's origin/destination nodes.
		parentDriver->origin.node = getParent()->originNode.node_;
		parentDriver->origin.point = parentDriver->origin.node->location;
		parentDriver->goal.node = getParent()->destNode.node_;
		parentDriver->goal.point = parentDriver->goal.node->location;

		if(parentDriver->origin.node == parentDriver->goal.node){
			Print()
			<< "DriverMovement::initializePath | Can't initializePath(); origin and destination are the same for driver " <<getParent()->GetId()
			<< "\norigin:" << parentDriver->origin.node->getID()
			<< "\ndestination:" << parentDriver->goal.node->getID()
			<< std::endl;
			return res;
		}

		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
		vector<WayPoint> path = getParent()->getCurrPath();
		if(path.empty()){
			//NOTE: I am fairly sure that the logic here is wrong; the parent is certainly not null, since
			//      we JUST called "getCurrPath()". The only reason this if statement was succeeding is that
			//      specialStr is always empty (we don't use it anymore). Since I am deleting specialStr,
			//      I commented out this entire "if" statement (although logically is would reduce to always false).
			//      I think I did the right thing. Please review. ~Seth.
			//if (!getParent() || getParent()->specialStr.empty()) {
                // if use path set
				if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
					path = PathSetManager::getInstance()->getPathByPerson(getParent());
				}
				else
				{
					const StreetDirectory& stdir = StreetDirectory::instance();
					path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*(parentDriver->origin).node), stdir.DrivingVertex(*(parentDriver->goal).node));
				}
				//const StreetDirectory& stdir = StreetDirectory::instance();
				//path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*(parentDriver->origin.node)), stdir.DrivingVertex(*(parentDriver->goal.node)));
			//}
			getParent()->setCurrPath(path);
		}
		//For now, empty paths aren't supported.
		if (path.empty()) {
			//throw std::runtime_error("Can't initializePath(); path is empty.");
			Print()<<"DriverMovement::initializePath | Can't initializePath(); path is empty for driver "
				   <<getParent()->GetId()<<std::endl;
			return res;
		}

		//TODO: Start in lane 0?
		int startlaneID = 0;

		// Bus should be at least 1200 to be displayed on Visualizer
		const double length = 400;
		const double width = 200;

		//A non-null vehicle means we are moving.
		if (allocateVehicle) {
			res = new Vehicle(path, startlaneID, length, width);
		}
	}
	//to indicate that the path to next activity is already planned
	getParent()->setNextPathPlanned(true);
	return res;
}

void DriverMovement::setParentData(DriverUpdateParams& p) {
	if(!vehicle->isDone()) {
		getParent()->distanceToEndOfSegment = vehicle->getPositionInSegment();
		getParent()->movingVelocity = vehicle->getVelocity();
		getParent()->setCurrLane(currLane);
		getParent()->setCurrSegment(vehicle->getCurrSegment());
		getParent()->setRemainingTimeThisTick(p.secondsInTick - p.elapsedSeconds);
	}
	else {
		getParent()->distanceToEndOfSegment = 0.0;
		getParent()->movingVelocity = 0.0;
		getParent()->setCurrLane(nullptr);
		getParent()->setCurrSegment(nullptr);
		getParent()->setRemainingTimeThisTick(0.0);
		getParent()->isQueuing = false;
	}
}

void DriverMovement::stepFwdInTime(DriverUpdateParams& p, double time) {
	p.elapsedSeconds = p.elapsedSeconds + time;
}

bool DriverMovement::advance(DriverUpdateParams& p) {
	if (vehicle->isDone()) {
		getParent()->setToBeRemoved();
		return false;
	}

	if(getParent()->getRemainingTimeThisTick() <= 0){
		return false;
	}

	if (vehicle->isQueuing)
	{
		return advanceQueuingVehicle(p);
	}
	else //vehicle is moving
	{
		return advanceMovingVehicle(p);
	}
}

bool DriverMovement::moveToNextSegment(DriverUpdateParams& p) {
	bool res = false;
	bool isNewLinkNext = ( !vehicle->hasNextSegment(true) && vehicle->hasNextSegment(false));
	const sim_mob::RoadSegment* nextRdSeg = nullptr;

	if (isNewLinkNext) { nextRdSeg = vehicle->getNextSegment(false); }
	else { nextRdSeg = vehicle->getNextSegment(true); }

	if (!nextRdSeg) {
		//vehicle is done
		vehicle->actualMoveToNextSegmentAndUpdateDir_med();
		if (vehicle->isDone()) {
			setOutputCounter(currLane, (getOutputCounter(currLane)-1));
			currLane = nullptr;
			getParent()->setToBeRemoved();
		}
		return false;
	}

	if(isNewLinkNext) {
		getParent()->requestedNextSegment = nextRdSeg;
		getParent()->canMoveToNextSegment = Person::NONE;
		return false; // return whenever a new link is to be entered. Seek permission from Conflux.
	}

	const sim_mob::RoadSegment* nextToNextRdSeg = vehicle->getSecondSegmentAhead();
	nextLaneInNextSegment = getBestTargetLane(nextRdSeg, nextToNextRdSeg);

	//this will space out the drivers on the same lane, by seperating them by the time taken for the previous car to move a car's length
	//Commenting out the delay from accept rate as per Yang Lu's suggestion (we only use this delay in setOrigin)
	double departTime = getLastAccept(nextLaneInNextSegment)/* + getAcceptRate(nextLaneInNextSegment)*/; //in seconds

	//skip acceptance capacity if there's no queue - this is done in DynaMIT
	//commenting out - the delay from acceptRate is removed as per Yang Lu's suggestion
/*	if(nextRdSeg->getParentConflux()->numQueueingInSegment(nextRdSeg, true) == 0){
		departTime = getLastAccept(nextLaneInNextSegment)
						+ (0.01 * vehicle->length) / (nextRdSeg->getParentConflux()->getSegmentSpeed(nextRdSeg, true) ); // skip input capacity
	}*/

	p.elapsedSeconds = std::max(p.elapsedSeconds, departTime - (p.now.ms()/1000.0)); //in seconds

	if (canGoToNextRdSeg(p)){
		if (vehicle->isQueuing){
			removeFromQueue();
		}

		setOutputCounter(currLane, (getOutputCounter(currLane)-1)); // decrement from the currLane before updating it
		currLane = nextLaneInNextSegment;
		vehicle->actualMoveToNextSegmentAndUpdateDir_med();
		vehicle->setPositionInSegment(nextRdSeg->getLaneZeroLength());
		double linkExitTimeSec =  p.elapsedSeconds + (p.now.ms()/1000.0);
		setLastAccept(currLane, linkExitTimeSec);

		if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
			const RoadSegment* prevSeg = vehicle->getPrevSegment(true);	//previous segment is in the same link
			if(prevSeg){
				// update road segment travel times
				updateRdSegTravelTimes(prevSeg, linkExitTimeSec);
			}
		}
		res = advance(p);
	}
	else{
		if (vehicle->isQueuing){
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

	DriverUpdateParams& p = dynamic_cast<DriverUpdateParams&>(up);

	const sim_mob::RoadSegment* nextRdSeg = vehicle->getNextSegment(false);
	const sim_mob::RoadSegment* nextToNextRdSeg = vehicle->getSecondSegmentAhead();
	nextLaneInNextSegment = getBestTargetLane(nextRdSeg, nextToNextRdSeg);

	//this will space out the drivers on the same lane, by seperating them by the time taken for the previous car to move a car's length
	//Commenting out the delay from accept rate as per Yang Lu's suggestion (we only use this delay in setOrigin)
	double departTime = getLastAccept(nextLaneInNextSegment)/* + getAcceptRate(nextLaneInNextSegment)*/; //in seconds

	p.elapsedSeconds = std::max(p.elapsedSeconds, departTime - (p.now.ms()/1000.0)); //in seconds

	if (canGoToNextRdSeg(p)){
		if (vehicle->isQueuing){
			removeFromQueue();
		}

		setOutputCounter(currLane, (getOutputCounter(currLane)-1));
		currLane = nextLaneInNextSegment;
		vehicle->actualMoveToNextSegmentAndUpdateDir_med();
		vehicle->setPositionInSegment(nextRdSeg->getLaneZeroLength());

		double linkExitTimeSec =  p.elapsedSeconds + (p.now.ms()/1000.0);
		//set Link Travel time for previous link
		const RoadSegment* prevSeg = vehicle->getPrevSegment(false);
		if (prevSeg) {
			// update link travel times
			updateLinkTravelTimes(prevSeg, linkExitTimeSec);

			if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
				// update road segment travel times
				updateRdSegTravelTimes(prevSeg, linkExitTimeSec);
			}
		}
		setLastAccept(currLane, linkExitTimeSec);
		setParentData(p);
		getParent()->canMoveToNextSegment = Person::NONE;
	}
	else {
		//Person is in previous segment (should be added to queue if canGoTo failed)
		if(vehicle->getCurrSegment() == getParent()->getCurrSegment() ){
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
		else if (vehicle->getNextSegment(false) == getParent()->getCurrSegment() ){
			//Person is in virtual queue (should remain in virtual queues if canGoTo failed)
			//do nothing
		}
		else{
			DebugStream << "Driver " << getParent()->getId()
					<< "was neither in virtual queue nor in previous segment!"
					<< "\nvehicle| segment: " << vehicle->getCurrSegment()->getStartEnd() << "|id: " << vehicle->getCurrSegment()->getSegmentID()
					<< "|lane: " << vehicle->getCurrLane()->getLaneID()
					<< "\ngetParent()| segment: " << getParent()->getCurrSegment()->getStartEnd() << "|id: " << getParent()->getCurrSegment()->getSegmentID()
					<< "|lane: " << (getParent()->getCurrLane()? getParent()->getCurrLane()->getLaneID():0)
					<< std::endl;

			throw::std::runtime_error(DebugStream.str());
		}
		p.elapsedSeconds = p.secondsInTick;
		getParent()->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
	}
}

bool DriverMovement::canGoToNextRdSeg(DriverUpdateParams& p) {
	//return false if the Driver cannot be added during this time tick
	if (p.elapsedSeconds >= p.secondsInTick) {
		return false;
	}

	//check if the next road segment has sufficient empty space to accommodate one more vehicle
	const RoadSegment* nextRdSeg = nextLaneInNextSegment->getRoadSegment();
	if (!nextRdSeg) {
		return false;
	}

	unsigned int total = vehicle->getCurrSegment()->getParentConflux()->numMovingInSegment(nextRdSeg, true)
						+ vehicle->getCurrSegment()->getParentConflux()->numQueueingInSegment(nextRdSeg, true);

	int vehLaneCount = nextRdSeg->getParentConflux()->getVehicleLaneCounts(nextRdSeg);
	double max_allowed = (vehLaneCount * nextRdSeg->getLaneZeroLength()/vehicle->length);
	return (total < max_allowed);
}

void DriverMovement::moveInQueue() {
	//1.update position in queue (vehicle->setPosition(distInQueue))
	//2.update p.timeThisTick
	double positionOfLastUpdatedAgentInLane = vehicle->getCurrSegment()->getParentConflux()->getPositionOfLastUpdatedAgentInLane(currLane);

	if(positionOfLastUpdatedAgentInLane == -1.0) {
		vehicle->setPositionInSegment(0.0);
	}
	else {
		vehicle->setPositionInSegment(positionOfLastUpdatedAgentInLane +  vehicle->length);
	}
}

bool DriverMovement::moveInSegment(DriverUpdateParams& p2, double distance) {
	double startPos = vehicle->getPositionInSegment();

	try {
		vehicle->moveFwd_med(distance);
	} catch (std::exception& ex) {
		std::stringstream msg;
		msg << "Error moving vehicle forward for Agent ID: " << getParent()->getId() << "," << this->vehicle->getPositionInSegment() << "\n" << ex.what();
		throw std::runtime_error(msg.str().c_str());
		return false;
	}

	double endPos = vehicle->getPositionInSegment();
	updateFlow(vehicle->getCurrSegment(), startPos, endPos);

	return true;
}

bool DriverMovement::advanceQueuingVehicle(DriverUpdateParams& p) {
	bool res = false;

	double t0 = p.elapsedSeconds;
	double x0 = vehicle->getPositionInSegment();
	double xf = 0.0;
	double tf = 0.0;

	double output = getOutputCounter(currLane);
	double outRate = getOutputFlowRate(currLane);
	tf = t0 + x0/(3.0*vehicle->length*outRate); //assuming vehicle length is in cm
	if (output > 0 && tf < p.secondsInTick &&
			currLane->getRoadSegment()->getParentConflux()->getPositionOfLastUpdatedAgentInLane(currLane) == -1)
	{
		res = moveToNextSegment(p);
		xf = vehicle->getPositionInSegment();
	}
	else
	{
		moveInQueue();
		xf = vehicle->getPositionInSegment();
		p.elapsedSeconds =  p.secondsInTick;
	}
	//unless it is handled previously;
	//1. update current position of vehicle/driver with xf
	//2. update current time, p.timeThisTick, with tf
	vehicle->setPositionInSegment(xf);

	return res;
}

bool DriverMovement::advanceMovingVehicle(DriverUpdateParams& p) {

	bool res = false;
	double t0 = p.elapsedSeconds;
	double x0 = vehicle->getPositionInSegment();
//	std::cout<<"rdSeg: "<<vehicle->getPositionInSegment()<<std::endl;
	double xf = 0.0;
	double tf = 0.0;

	if(!currLane) {
		throw std::runtime_error("agent's current lane is not set!");
	}

	getSegSpeed();

	double vu = vehicle->getVelocity();

	double output = getOutputCounter(currLane);

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
			vehicle->setPositionInSegment(xf);
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
				vehicle->setPositionInSegment(0.0);
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
			vehicle->setPositionInSegment(xf);
			p.elapsedSeconds = tf;
		}
	}
	return res;
}

bool DriverMovement::advanceMovingVehicleWithInitialQ(DriverUpdateParams& p) {

	bool res = false;
	double t0 = p.elapsedSeconds;
	double x0 = vehicle->getPositionInSegment();
	double xf = 0.0;
	double tf = 0.0;

	getSegSpeed();
	double vu = vehicle->getVelocity();

	double output = getOutputCounter(currLane);
	double outRate = getOutputFlowRate(currLane);

	double timeToDissipateQ = getInitialQueueLength(currLane)/(3.0*outRate*vehicle->length); //assuming vehicle length is in cm
	double timeToReachEndSeg = t0 + x0/vu;
	tf = std::max(timeToDissipateQ, timeToReachEndSeg);

	if (tf < p.secondsInTick)
	{
		if (output > 0)
		{
			vehicle->setPositionInSegment(0.0);
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

		vehicle->setPositionInSegment(xf);
		p.elapsedSeconds = tf;
	}
	return res;
}

void DriverMovement::getSegSpeed() {
	vehicle->setVelocity(vehicle->getCurrSegment()->
			getParentConflux()->getSegmentSpeed(vehicle->getCurrSegment(), true));
}

int DriverMovement::getOutputCounter(const Lane* l) {
	return getParent()->getCurrSegment()->getParentConflux()->getOutputCounter(l);
}

void DriverMovement::setOutputCounter(const Lane* l, int count) {
	return getParent()->getCurrSegment()->getParentConflux()->setOutputCounter(l, count);
}

double DriverMovement::getOutputFlowRate(const Lane* l) {
	return getParent()->getCurrSegment()->getParentConflux()->getOutputFlowRate(l);
}

double DriverMovement::getAcceptRate(const Lane* l) {
	return getParent()->getCurrSegment()->getParentConflux()->getAcceptRate(l);
}

double DriverMovement::getQueueLength(const Lane* l) {
	return ((getParent()->getCurrSegment()->getParentConflux()->getLaneAgentCounts(l)).first) * (vehicle->length);
}

double DriverMovement::getLastAccept(const Lane* l) {
	return getParent()->getCurrSegment()->getParentConflux()->getLastAccept(l);
}

void DriverMovement::setLastAccept(const Lane* l, double lastAccept) {
	getParent()->getCurrSegment()->getParentConflux()->setLastAccept(l, lastAccept);
}

void DriverMovement::updateFlow(const RoadSegment* rdSeg, double startPos, double endPos) {
	//double mid = rdSeg->computeLaneZeroLength()/2.0;
	double mid = rdSeg->getLaneZeroLength()/2.0;
	if (startPos >= mid && mid >= endPos){
		rdSeg->getParentConflux()->incrementSegmentFlow(rdSeg);
	}
}

void DriverMovement::setOrigin(DriverUpdateParams& p) {

	//Vehicles start at rest
	vehicle->setVelocity(0);

	if(p.now.ms() < getParent()->getStartTime()) {
		stepFwdInTime(p, (getParent()->getStartTime() - p.now.ms())/1000.0); //set time to start - to accommodate drivers starting during the frame
	}

	const sim_mob::RoadSegment* nextRdSeg = nullptr;
	if (vehicle->hasNextSegment(true)) {
		nextRdSeg = vehicle->getNextSegment(true);
	}
	else if (vehicle->hasNextSegment(false)) {
		nextRdSeg = vehicle->getNextSegment(false);
	}

	nextLaneInNextSegment = getBestTargetLane(vehicle->getCurrSegment(), nextRdSeg);

	//this will space out the drivers on the same lane, by seperating them by the time taken for the previous car to move a car's length
	double departTime = getLastAccept(nextLaneInNextSegment) + getAcceptRate(nextLaneInNextSegment); //in seconds

	/*//skip acceptance capacity if there's no queue - this is done in DynaMIT
	if(vehicle->getCurrSegment()->getParentConflux()->numQueueingInSegment(vehicle->getCurrSegment(), true) == 0){
		departTime = getLastAccept(nextLaneInNextSegment)
						+ (0.01 * vehicle->length) / (vehicle->getCurrSegment()->getParentConflux()->getSegmentSpeed(vehicle->getCurrSegment(), true) ); // skip input capacity
	}*/

	p.elapsedSeconds = std::max(p.elapsedSeconds, departTime - (p.now.ms()/1000.0));	//in seconds

	if(canGoToNextRdSeg(p))
	{
		//set position to start
		if(vehicle->getCurrSegment())
		{
			vehicle->setPositionInSegment(vehicle->getCurrSegment()->getLaneZeroLength());
		}
		currLane = nextLaneInNextSegment;
		double actualT = p.elapsedSeconds + (p.now.ms()/1000.0);
		getParent()->initLinkTravelStats(vehicle->getCurrSegment()->getLink(), actualT);

		setLastAccept(currLane, actualT);
		setParentData(p);
		getParent()->canMoveToNextSegment = Person::NONE;
	}
	else
	{
		p.elapsedSeconds = p.secondsInTick;
		getParent()->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
	}
}

bool DriverMovement::isConnectedToNextSeg(const Lane* lane, const RoadSegment* nextRdSeg) {
	if( !nextRdSeg)
		throw std::runtime_error("nextRdSeg is not available!");

	if (nextRdSeg->getLink() != lane->getRoadSegment()->getLink()){
		const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (lane->getRoadSegment()->getEnd());
		if (currEndNode && nextRdSeg) {
			const set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes((lane->getRoadSegment()));
			for (set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++) {
				if ((*it)->getLaneTo()->getRoadSegment() == nextRdSeg && (*it)->getLaneFrom() == lane) {
					return true;
				}
			}
		}
	}
	else{ //handling uni-nodes - where lanes should be connected to the same outgoing segment
		if (lane->getRoadSegment()->getLink() == nextRdSeg->getLink())
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
			vehicle->setPositionInSegment(getQueueLength(lane));
			vehicle->isQueuing = true;
			parentP->isQueuing = vehicle->isQueuing;
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
			vehicle->isQueuing = false;
		}
		else {
			 DebugStream << "removeFromQueue() was called for a driver who is not in queue. Person: " << parentP->getId()
					<< "|RoadSegment: " << currLane->getRoadSegment()->getStartEnd()
					<< "|Lane: " << currLane->getLaneID() << std::endl;
			 throw std::runtime_error(DebugStream.str());
		}
	}
}

const sim_mob::Lane* DriverMovement::getBestTargetLane(const RoadSegment* nextRdSeg, const RoadSegment* nextToNextRdSeg) {
	//we have included getBastLG functionality here (get lane with minAllAgents)
	//before checking best lane
	//1. Get queueing counts for all lanes of the next Segment
	//2. Select the lane with the least queue length
	//3. Update nextLaneInNextLink and targetLaneIndex accordingly
	if(!nextRdSeg) {
		return nullptr;
	}

	const sim_mob::Lane* minQueueLengthLane = nullptr;
	const sim_mob::Lane* minAgentsLane = nullptr;
	unsigned int minQueueLength = std::numeric_limits<int>::max();
	unsigned int minAllAgents = std::numeric_limits<int>::max();
	unsigned int que = 0;
	unsigned int total = 0;
	int test_count = 0;

	vector<sim_mob::Lane* >::const_iterator i = nextRdSeg->getLanes().begin();
	vector<sim_mob::Lane* > laneGroup;	//temporary container to save the connected lanes

	//getBestLaneGroup logic
	for ( ; i != nextRdSeg->getLanes().end(); ++i){
		if ( !((*i)->is_pedestrian_lane())){
			if(nextToNextRdSeg && !isConnectedToNextSeg(*i, nextToNextRdSeg))	{
				continue;
			}
			laneGroup.push_back(*i);
			std::pair<unsigned int, unsigned int> counts = vehicle->getCurrSegment()->getParentConflux()->getLaneAgentCounts(*i); //<Q,M>
			que = counts.first;
			total = que + counts.second;

			if (minAllAgents > total){
				minAllAgents = total;
				minAgentsLane = *i;
			}
		}
	}

	//getBestLane logic
	for (i = laneGroup.begin(); i != laneGroup.end(); ++i){
		std::pair<unsigned int, unsigned int> counts = vehicle->getCurrSegment()->getParentConflux()->getLaneAgentCounts(*i); //<Q, M>
			que = counts.first;
			total = que + counts.second;
			if (minAllAgents == total){
				if (minQueueLength > que){
					minQueueLength = que;
					minQueueLengthLane = *i;
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
	return getParent()->getCurrSegment()->getParentConflux()->getInitialQueueCount(l) * vehicle->length;
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

void DriverMovement::updateLinkTravelTimes(const RoadSegment* prevSeg, double linkExitTimeSec){
	const Link* prevLink = prevSeg->getLink();
	if(prevLink == getParent()->getLinkTravelStats().link_){
		getParent()->addToLinkTravelStatsMap(getParent()->getLinkTravelStats(), linkExitTimeSec); //in seconds
		prevSeg->getParentConflux()->setLinkTravelTimes(getParent(), linkExitTimeSec);
	}
	//creating a new entry in agent's travelStats for the new link, with entry time
	getParent()->initLinkTravelStats(vehicle->getCurrSegment()->getLink(), linkExitTimeSec);
}

void DriverMovement::updateRdSegTravelTimes(const RoadSegment* prevSeg, double linkExitTimeSec){
	//if prevSeg is already in travelStats, update it's rdSegTT and add to rdSegTravelStatsMap
	if(prevSeg == getParent()->getRdSegTravelStats().rdSeg_){
		getParent()->addToRdSegTravelStatsMap(getParent()->getRdSegTravelStats(), linkExitTimeSec); //in seconds
		prevSeg->getParentConflux()->setRdSegTravelTimes(getParent(), linkExitTimeSec);
	}
	//creating a new entry in agent's travelStats for the new road segment, with entry time
	getParent()->initRdSegTravelStats(vehicle->getCurrSegment(), linkExitTimeSec);
}

} /* namespace medium */
} /* namespace sim_mob */
