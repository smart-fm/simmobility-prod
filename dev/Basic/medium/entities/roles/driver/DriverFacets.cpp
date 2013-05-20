/*
 * DriverFacets.cpp
 *
 *  Created on: Apr 1, 2013
 *      Author: harish
 */

#include "DriverFacets.hpp"
#include <cmath>
#include <ostream>
#include <algorithm>

#include "entities/Person.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/conflux/Conflux.hpp"

#include "buffering/BufferedDataManager.hpp"
#include "conf/simpleconf.hpp"

#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Point2D.hpp"

#include "util/OutputUtil.hpp"
#include "util/DebugFlags.hpp"

#include "partitions/PartitionManager.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;

using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;

namespace sim_mob {
namespace medium {

DriverBehavior::DriverBehavior(sim_mob::Person* parentAgent):
	BehaviorFacet(parentAgent), parentDriver(nullptr) {}

DriverBehavior::~DriverBehavior() {}

void DriverBehavior::frame_init(UpdateParams& p) {
	throw std::runtime_error("DriverBehavior::frame_init is not implemented yet");
}

void DriverBehavior::frame_tick(UpdateParams& p) {
	throw std::runtime_error("DriverBehavior::frame_tick is not implemented yet");
}

void DriverBehavior::frame_tick_output(const UpdateParams& p) {
	throw std::runtime_error("DriverBehavior::frame_tick_output is not implemented yet");
}

void DriverBehavior::frame_tick_output_mpi(timeslice now) {
	throw std::runtime_error("DriverBehavior::frame_tick_output_mpi is not implemented yet");
}

sim_mob::medium::DriverMovement::DriverMovement(sim_mob::Person* parentAgent):
	MovementFacet(parentAgent), parentDriver(nullptr), vehicle(nullptr), currLane(nullptr), nextLaneInNextSegment(nullptr) {}

sim_mob::medium::DriverMovement::~DriverMovement() {}

void sim_mob::medium::DriverMovement::frame_init(UpdateParams& p) {
	//Save the path from orign to next activity location in allRoadSegments
//	if (!parentDriver->getResource()) {
//		Vehicle* veh = initializePath(true);
//		if (veh) {
//			safe_delete_item(vehicle);
//			//To Do: Better to use currResource instead of vehicle, when handling other roles ~melani
//			vehicle = veh;
//			parentDriver->setResource(veh);
//		}
//	}
//	else {
//		initializePath(false);
//	}

	Vehicle* newVeh = initializePath(true);
	if (newVeh) {
		safe_delete_item(vehicle);
		vehicle = newVeh;
		parentDriver->setResource(newVeh);
	}
	Print() << "DriverMovement::frame_init|Frame#: " << p.now.frame() << "|Person: " << parentAgent->getId()
			<< "|First segment in path: " << newVeh->getCurrSegment()->getStartEnd() << "|First sgmt id: " << newVeh->getCurrSegment()->getSegmentID()
			<< std::endl;
}

void sim_mob::medium::DriverMovement::frame_tick(UpdateParams& p) {
	DriverUpdateParams& p2 = dynamic_cast<DriverUpdateParams&>(p);

	const Lane* laneInfinity = nullptr;
	laneInfinity = vehicle->getCurrSegment()->getParentConflux()->getLaneInfinity(vehicle->getCurrSegment());

	if(vehicle->getCurrSegment() == parentAgent->getCurrSegment() )
	{
		if (vehicle->hasPath() && laneInfinity)
		{
			//at start vehicle will be in lane infinity. set origin will move it to the correct lane
			if (parentAgent->getCurrLane() == laneInfinity){
				Print() <<"DriverMovement::frame_tick|Frame#: " << p.now.frame() << "|Person: " << parentAgent->getId()
						<<"|calling setOrigin "<< vehicle->getCurrSegment()->getStartEnd() <<std::endl;
				//setOrigin(parentDriver->params);
				setOrigin(p2);
			}
		} else {
			LogOut("ERROR: Vehicle could not be created for driver; no route!" <<std::endl);
		}
	}

	//Are we done already?
	if (vehicle->isDone()) {
		parentAgent->setToBeRemoved();
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
	Print() << "DriverMovement::frame_tick|Frame#: " << p2.now.frame() << "|Person: " << parentAgent->getId();
	if(parentAgent->canMoveToNextSegment == Person::GRANTED) {
		flowIntoNextLinkIfPossible(p2);
	}
	else if (parentAgent->canMoveToNextSegment == Person::DENIED){
		Print() << "|Permission: Denied"
				<< "|vehicle|CurrSegment:" << vehicle->getCurrSegment()->getStartEnd() << "|CurrLane:" << vehicle->getCurrLane()->getLaneID()
				<< "|Person|CurrSegment:" << parentAgent->getCurrSegment()->getStartEnd() << "|CurrLane:" << parentAgent->getCurrLane()->getLaneID()
				<< std::endl;

		if(currLane) {
			if(parentAgent->isQueuing) {
				moveInQueue();
				Print() << "|called moveInQueue()";
			}
			else {
				addToQueue(currLane); // adds to queue if not already in queue
				Print() << "|called addToQueue()";
			}

			p2.elapsedSeconds = p2.secondsInTick;
			parentAgent->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
		}
	}
	//if vehicle is still in lane infinity, it shouldn't be advanced
	if (currLane && parentAgent->canMoveToNextSegment == Person::NONE)
	{
		advance(p2);
		//Update parent data. Only works if we're not "done" for a bad reason.
		setParentData(p2);
		Print() << "|Frame#: " << p2.now.frame() << "|Person: " << parentAgent->getId() << "|called advance() and setParentData()" << std::endl;
	}
	else {
		Print() << "|Frame#: " << p2.now.frame() << "|Person: " << parentAgent->getId() << "|missed advance() and setParentData()"
				<< "|canMoveToNextSegment: " << parentAgent->canMoveToNextSegment << std::endl;
	}
}

void sim_mob::medium::DriverMovement::frame_tick_output(const UpdateParams& p) {
	//Skip?
	if (vehicle->isDone() || ConfigParams::GetInstance().is_run_on_many_computers || ConfigParams::GetInstance().OutputDisabled()) {
		return;
	}

	std::stringstream logout;
	logout << "(\"Driver\""
			<<","<<parentAgent->getId()
			<<","<<p.now.frame()
			<<",{"
			<<"\"RoadSegment\":\""<< (parentAgent->getCurrSegment()->getSegmentID())
			<<"\",\"Lane\":\""<<((parentAgent->getCurrLane())? parentAgent->getCurrLane()->getLaneID():0)
			<<"\",\"UpNode\":\""<<(parentAgent->getCurrSegment()->getStart()->getID())
			<<"\",\"DistanceToEndSeg\":\""<<parentAgent->distanceToEndOfSegment;
	if (this->parentAgent->isQueuing) {
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
	if(!parentAgent->getNextPathPlanned()){
		//Save local copies of the parent's origin/destination nodes.
		parentDriver->origin.node = parentAgent->originNode.node_;
		parentDriver->origin.point = parentDriver->origin.node->location;
		parentDriver->goal.node = parentAgent->destNode.node_;
		parentDriver->goal.point = parentDriver->goal.node->location;

		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
		vector<WayPoint> path;
		if (!parentAgent || parentAgent->specialStr.empty()) {
			const StreetDirectory& stdir = StreetDirectory::instance();
			path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*(parentDriver->origin.node)), stdir.DrivingVertex(*(parentDriver->goal.node)));
		}

		//For now, empty paths aren't supported.
		if (path.empty()) {
			throw std::runtime_error("Can't initializePath(); path is empty.");
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
	parentAgent->setNextPathPlanned(true);
	return res;
}

void DriverMovement::setParentData(DriverUpdateParams& p) {
		if(!vehicle->isDone()) {
			parentAgent->distanceToEndOfSegment = vehicle->getPositionInSegment();
			parentAgent->movingVelocity = vehicle->getVelocity();
			parentAgent->setCurrLane(currLane);
			parentAgent->setCurrSegment(vehicle->getCurrSegment());
			parentAgent->setRemainingTimeThisTick(p.secondsInTick - p.elapsedSeconds);
		}
		else {
			parentAgent->distanceToEndOfSegment = 0.0;
			parentAgent->movingVelocity = 0.0;
			parentAgent->setCurrLane(nullptr);
			parentAgent->setCurrSegment(nullptr);
			parentAgent->setRemainingTimeThisTick(0.0);
		}
}

double DriverMovement::getTimeSpentInTick(DriverUpdateParams& p) {
	return p.elapsedSeconds;
}

void DriverMovement::stepFwdInTime(DriverUpdateParams& p, double time) {
	p.elapsedSeconds = p.elapsedSeconds + time;
}

bool DriverMovement::advance(DriverUpdateParams& p) {
	if (vehicle->isDone()) {
		parentAgent->setToBeRemoved();
		return false;
	}

	if(parentAgent->getRemainingTimeThisTick() <= 0){
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
			currLane = nullptr;
			parentAgent->setToBeRemoved();
		}
		return false;
	}

	if(isNewLinkNext) {
		parentAgent->requestedNextSegment = nextRdSeg;
		parentAgent->canMoveToNextSegment = Person::NONE;
		return false; // return whenever a new link is to be entered. Seek permission from Conflux.
	}

	const sim_mob::RoadSegment* nextToNextRdSeg = vehicle->getSecondSegmentAhead();
	nextLaneInNextSegment = getBestTargetLane(nextRdSeg, nextToNextRdSeg);

	double departTime = getLastAccept(nextLaneInNextSegment) + getAcceptRate(nextLaneInNextSegment); //in seconds
	p.elapsedSeconds = std::max(p.elapsedSeconds, departTime - (p.now.ms()/1000.0)); //in seconds

	if (canGoToNextRdSeg(p, p.elapsedSeconds)){
		if (vehicle->isQueuing){
			removeFromQueue();
		}
		else{
		//	removeFromMovingList();
		}

		currLane = nextLaneInNextSegment;
		vehicle->actualMoveToNextSegmentAndUpdateDir_med();
		vehicle->setPositionInSegment(vehicle->getCurrLinkLaneZeroLength());

		double linkExitTimeSec =  p.elapsedSeconds + (p.now.ms()/1000.0);

		/*if (isNewLinkNext)
		{
			//set Link Travel time for previous link
			const RoadSegment* prevSeg = vehicle->getPrevSegment(false);
			if (prevSeg){
				const Link* prevLink = prevSeg->getLink();
				//if prevLink is already in travelStats, update it's linkTT and add to travelStatsMap
				if(prevLink == parentAgent->getTravelStats().link_){
					parentAgent->addToTravelStatsMap(parentAgent->getTravelStats(), linkExitTimeSec); //in seconds
					prevSeg->getParentConflux()->setTravelTimes(parentAgent, linkExitTimeSec);
				}
				//creating a new entry in agent's travelStats for the new link, with entry time
				parentAgent->initTravelStats(vehicle->getCurrSegment()->getLink(), linkExitTimeSec);
			}
		}
*/
		/*	std::cout<< parent->getId()<<" Driver is movedToNextSeg at: "<< linkExitTimeSec*1000 << "ms to lane "
		 	 	 	<< currLane->getLaneID_str()
					<<" in RdSeg "<< vehicle->getCurrSegment()->getStart()->getID()
					<<" last Accept: "<< getLastAccept(currLane)
					<<" accept rate: "<<getAcceptRate(currLane)
					<<" timeThisTick: "<<p.timeThisTick
					<<" now: "<<p.now.ms()
					<< " dist2End: "<<vehicle->getPositionInSegment()
					<< std::endl;*/

			setLastAccept(currLane, linkExitTimeSec);
			res = advance(p);
	}
	else{
		Print() <<"moveToNextRdSeg | canGoTo failed!"<<std::endl;
		if (vehicle->isQueuing){
			moveInQueue();
		}
		else{
			addToQueue(currLane);
		}
		p.elapsedSeconds = p.secondsInTick;
		parentAgent->setRemainingTimeThisTick(0.0);
	}
	return res;
}

void DriverMovement::flowIntoNextLinkIfPossible(UpdateParams& up) {
	//This function gets called for 2 cases.
	//1. Driver is added to virtual queue
	//2. Driver is in previous segment trying to add to the next

	Print()	<< "|Permission: Granted"
			<< "|vehicle|CurrSegment:" << vehicle->getCurrSegment()->getStartEnd() << "|CurrLane:" << vehicle->getCurrLane()->getLaneID()
			<< "|Person|CurrSegment:" << parentAgent->getCurrSegment()->getStartEnd() << "|CurrLane:" << parentAgent->getCurrLane()->getLaneID()
			<< std::endl;

	DriverUpdateParams& p = dynamic_cast<DriverUpdateParams&>(up);

	const sim_mob::RoadSegment* nextRdSeg = vehicle->getNextSegment(false);
	const sim_mob::RoadSegment* nextToNextRdSeg = vehicle->getSecondSegmentAhead();
	nextLaneInNextSegment = getBestTargetLane(nextRdSeg, nextToNextRdSeg);

	double departTime = getLastAccept(nextLaneInNextSegment) + getAcceptRate(nextLaneInNextSegment); //in seconds
	p.elapsedSeconds = std::max(p.elapsedSeconds, departTime - (p.now.ms()/1000.0)); //in seconds

	if (canGoToNextRdSeg(p, p.elapsedSeconds)){
		if (vehicle->isQueuing){
			removeFromQueue();
		}

		currLane = nextLaneInNextSegment;
		vehicle->actualMoveToNextSegmentAndUpdateDir_med();
		vehicle->setPositionInSegment(vehicle->getCurrLinkLaneZeroLength());

		double linkExitTimeSec =  p.elapsedSeconds + (p.now.ms()/1000.0);
		//set Link Travel time for previous link
		const RoadSegment* prevSeg = vehicle->getPrevSegment(false);
		if (prevSeg){
			const Link* prevLink = prevSeg->getLink();
			//if prevLink is already in travelStats, update it's linkTT and add to travelStatsMap
			if(prevLink == parentAgent->getTravelStats().link_){
				parentAgent->addToTravelStatsMap(parentAgent->getTravelStats(), linkExitTimeSec); //in seconds
				prevSeg->getParentConflux()->setTravelTimes(parentAgent, linkExitTimeSec);
			}
		//creating a new entry in agent's travelStats for the new link, with entry time
		parentAgent->initTravelStats(vehicle->getCurrSegment()->getLink(), linkExitTimeSec);
		}

		Print() << "DriverMovement::flowIntoNextLinkIfPossible|Frame#: " << p.now.frame() << "|Person: " << parentAgent->getId()
				<< "|canGoToNextRdSeg successful"
				<< "|Driver is movedToNextLink at: " << linkExitTimeSec*1000 << "ms"
				<< "|moved to nextRdSeg: " << vehicle->getCurrSegment()->getStartEnd() << "|nextRdSeg id: " << vehicle->getCurrSegment()->getSegmentID()
				<< "|lane: " << currLane->getLaneID()
				<< "|last Accept: "<< getLastAccept(currLane)
				<< "|accept rate: "<<getAcceptRate(currLane)
				<< "|elapsedSeconds: " << p.elapsedSeconds
				<< "|dist2End: " <<vehicle->getPositionInSegment()
				<< std::endl;

		setLastAccept(currLane, linkExitTimeSec);
		setParentData(p);
		parentAgent->canMoveToNextSegment = Person::NONE;
	}
	else {
		Print() << "DriverMovement::flowIntoNextLinkIfPossible|Frame#: " << p.now.frame() << "|Person: " << parentAgent->getId()
				<< "|canGoToNextRdSeg failed"
				<< "|nextRdSeg: " << nextRdSeg->getStartEnd() << "|nextRdSeg id: " << nextRdSeg->getSegmentID()
				<< "|lane: " << currLane->getLaneID()
				<< "|last Accept: "<< getLastAccept(nextLaneInNextSegment)
				<< "|accept rate: "<< getAcceptRate(nextLaneInNextSegment)
				<< "|elapsedSeconds: " << p.elapsedSeconds
				<< "|dist2End: " << vehicle->getPositionInSegment();

		//Person is in previous segment (should be added to queue if canGoTo failed)
		if(vehicle->getCurrSegment() == parentAgent->getCurrSegment() ){
			Print() << "|currSegment" << vehicle->getCurrSegment()->getStartEnd()
					<< "|isQueuing:" << parentAgent->isQueuing;
			if(currLane){
				if(parentAgent->isQueuing) {
					moveInQueue();
					Print() << "|called moveInQueue()"<< std::endl;
				}
				else {
					addToQueue(currLane); // adds to queue if not already in queue
					Print() << "|called addToQueue()"<< std::endl;
				}

				parentAgent->canMoveToNextSegment = Person::NONE; // so that advance() and setParentData() is called subsequently
			}
		}
		//Person is in virtual queue (should remain in virtual queues if canGoTo failed)
		else if (vehicle->getNextSegment(false) == parentAgent->getCurrSegment() ){
			Print() << "Driver remains in virtual queue a.k.a. lane infinity" << std::endl;
		}
		else{
			DebugStream << "Driver " << parentAgent->getId()
					<< "was neither in Virtual queue nor in previous segment!"
					<< "\nvehicle| segment: " << vehicle->getCurrSegment()->getStartEnd() << "|id: " << vehicle->getCurrSegment()->getSegmentID()
					<< "|lane: " << vehicle->getCurrLane()->getLaneID()
					<< "\nparentAgent| segment: " << parentAgent->getCurrSegment()->getStartEnd() << "|id: " << parentAgent->getCurrSegment()->getSegmentID()
					<< "|lane: " << parentAgent->getCurrLane()->getLaneID()
					<< std::endl;

			throw::std::runtime_error(DebugStream.str());
		}
		Print() << std::endl;
		p.elapsedSeconds = p.secondsInTick;
		parentAgent->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
		//parentAgent->canMoveToNextSegment = Person::DENIED;
	}
}

bool DriverMovement::canGoToNextRdSeg(DriverUpdateParams& p, double t) {
	//return false if the Driver cannot be added during this time tick
	if (t >= p.secondsInTick) {
		return false;
	}

	//check if the next road segment has sufficient empty space to accommodate one more vehicle
	const RoadSegment* nextRdSeg = nextLaneInNextSegment->getRoadSegment();
	if (!nextRdSeg) {
		return false;
	}

	unsigned int total = vehicle->getCurrSegment()->getParentConflux()->numMovingInSegment(nextRdSeg, true)
						+ vehicle->getCurrSegment()->getParentConflux()->numQueueingInSegment(nextRdSeg, true);

	int vehLaneCount = 0;
	for(std::vector<sim_mob::Lane*>::const_iterator laneIt=nextRdSeg->getLanes().begin(); laneIt!=nextRdSeg->getLanes().end(); laneIt++)
	{
		if (!(*laneIt)->is_pedestrian_lane()) { vehLaneCount += 1; }
	}

	/*DebugStream << "Frame: " << p.now.frame()
				<< "| nextRdSeg: ["<<nextRdSeg->getStart()->getID()<<","<<nextRdSeg->getEnd()->getID()<<"]"
				<<" | queueCount: " << vehicle->getCurrSegment()->getParentConflux()->numQueueingInSegment(nextRdSeg, true)
				<<" | movingCount: "<<vehicle->getCurrSegment()->getParentConflux()->numMovingInSegment(nextRdSeg, true)
				<<" | numLanes: " << nextRdSeg->getLanes().size()
				<<" | physical cap: " << vehLaneCount * nextRdSeg->computeLaneZeroLength()/vehicle->length - total
				<<" | length: " << nextRdSeg->computeLaneZeroLength()
				<<" | empty space: "<< (vehLaneCount * nextRdSeg->computeLaneZeroLength())-(total*vehicle->length)
				<<std::endl;*/

	double max_allowed = (vehLaneCount * nextRdSeg->computeLaneZeroLength()/vehicle->length);
/*	Print() << "|canGoToNextRdSeg"
			<< "|total: " << total
			<< "|max_allowed: " << max_allowed;*/

	return total < max_allowed;

	//return total - (vehLaneCount * nextRdSeg->computeLaneZeroLength()/vehicle->length)
	//	< std::numeric_limits<double>::epsilon( );
	//we use following in place of checking if veh length is less than or equal to empty space
	//return vehicle->length - (vehLaneCount * nextRdSeg->computeLaneZeroLength())
	//	- (total*vehicle->length) < std::numeric_limits<double>::epsilon( ) ;
}

void DriverMovement::moveInQueue() {
	//1.update position in queue (vehicle->setPosition(distInQueue))
	//2.update p.timeThisTick
	double positionOfLastUpdatedAgentInLane = parentAgent->getCurrSegment()->getParentConflux()->getPositionOfLastUpdatedAgentInLane(parentAgent->getCurrLane());

	if(positionOfLastUpdatedAgentInLane == -1.0)
	{
		vehicle->setPositionInSegment(0.0);
	}
	else
	{
		vehicle->setPositionInSegment(positionOfLastUpdatedAgentInLane +  vehicle->length);
	}
}

bool DriverMovement::moveInSegment(DriverUpdateParams& p2, double distance) {
	double startPos = vehicle->getPositionInSegment();

	try {
		vehicle->moveFwd_med(distance);
	} catch (std::exception& ex) {
		if (Debug::Drivers) {
			if (ConfigParams::GetInstance().OutputEnabled()) {
				DebugStream << ">>>Exception: " << ex.what() << endl;
				Print()<<(DebugStream.str());
			}
		}

		std::stringstream msg;
		msg << "Error moving vehicle forward for Agent ID: " << parentAgent->getId() << ","
				<< this->vehicle->getPositionInSegment() << "\n" << ex.what();
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
	tf = t0 + x0/(vehicle->length*outRate); //assuming vehicle length is in cm
	if (output > 0 && tf < p.secondsInTick)
	{
		res = moveToNextSegment(p);
		xf = vehicle->getPositionInSegment();
	}
	else
	{
		moveInQueue();
		xf = vehicle->getPositionInSegment();
		//tf = p.secondsInTick;
		p.elapsedSeconds =  p.secondsInTick; //harish 20-May
	}
	//unless it is handled previously;
	//1. update current position of vehicle/driver with xf
	//2. update current time, p.timeThisTick, with tf
	vehicle->setPositionInSegment(xf);
//	std::cout<<"advanceQueuingVehicle rdSeg: "<<vehicle->getCurrSegment()->getStart()->getID()<<" setPos: "<< xf<<std::endl;

	//p.elapsedSeconds = tf; commented by harish 20-May

	return res;
}

bool DriverMovement::advanceMovingVehicle(DriverUpdateParams& p) {

	bool res = false;
	double t0 = p.elapsedSeconds;
	double x0 = vehicle->getPositionInSegment();
//	std::cout<<"rdSeg: "<<vehicle->getPositionInSegment()<<std::endl;
	double xf = 0.0;
	double tf = 0.0;

	if(!currLane)
		throw std::runtime_error("agent's current lane is not set!");

	getSegSpeed();

	double vu = vehicle->getVelocity();

	double output = getOutputCounter(currLane);

	//get current location
	//before checking if the vehicle should be added to a queue, it's re-assigned to the best lane
	double laneQueueLength = getQueueLength(currLane);
//	std::cout << "queue length: " << laneQueueLength << " | seg length: "<<
//			vehicle->getCurrLinkLaneZeroLength() << std::endl;
	if (laneQueueLength > vehicle->getCurrLinkLaneZeroLength() )
	{
//		std::cout<< "queue longer than segment"<<vehicle->getCurrSegment()->getStart()->getID()<< std::endl;
		addToQueue(currLane);
		p.elapsedSeconds = p.secondsInTick;
	}
	else if (laneQueueLength > 0)
	{
/*		std::cout<<parent->getId() << " has queue: lane: "
								<<currLane->getLaneID_str()<<
								" segment: "<<vehicle->getCurrSegment()->getStart()->getID() <<std::endl;
		std::cout<<"time: "<<p.now.ms() <<
				" currLane: "<<currLane->getLaneID_str() << " queue length: "
								<<getQueueLength(currLane) <<std::endl;*/
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
//			std::cout<<"advanceMoving (with Q) rdSeg: "<<vehicle->getf()->getStart()->getID()<<" setPos: "<< xf<<std::endl;
			vehicle->setPositionInSegment(xf);
			p.elapsedSeconds = p.secondsInTick;
		}
	}
	else if (getInitialQueueLength(currLane) > 0)
	{
//		std::cout<< "has initial queue"<< std::endl;
		res = advanceMovingVehicleWithInitialQ(p);
	}
	else //no queue or no initial queue
	{
//		std::cout << "no queue" << std::endl;
		tf = t0 + x0/vu;
		if (tf < p.secondsInTick)
		{
/*			ss << vehicle->getCurrSegment()->getStart()->getID()
					<<"tf less than tick | output: " << output << endl;
			std::cout<<ss.str();
			ss.str("");*/

			if (output > 0)
			{
				vehicle->setPositionInSegment(0.0);
				p.elapsedSeconds = tf;
				res = moveToNextSegment(p);
			}
			else
			{
/*				std::cout<<parent->getId() << " add to queue: "
						<<" start Seg:"<<vehicle->getCurrSegment()->getStart()->getID()
						<<" currLane: "<<currLane->getLaneID_str()<<std::endl;*/
				addToQueue(currLane);
/*				std::cout<<currLane->getLaneID_str() << " queue length: "
						<<getQueueLength(currLane) <<std::endl;*/
				p.elapsedSeconds = p.secondsInTick;
			}
		}
		else
		{
//			std::cout << "tf more than tick" << std::endl;
			tf = p.secondsInTick;
			xf = x0-vu*(tf-t0);
			res = moveInSegment(p, x0-xf);
			vehicle->setPositionInSegment(xf);
//			std::cout<<"advanceMovingVehicle(no Q) rdSeg: "<<vehicle->getCurrSegment()->getStart()->getID()<<" setPos: "<<xf<<std::endl;

/*			std::cout<<"new position in advance: "<< vehicle->getPositionInSegment()
					<<" for segment "<<vehicle->getCurrSegment()->getStart()->getID()
					<<" veh lane: "<<vehicle->getCurrLane()->getLaneID_str()
					<<" lane: "<<currLane->getLaneID_str()
					<<std::endl;*/
			p.elapsedSeconds = tf;
			//p2.currLaneOffset = vehicle->getDistanceMovedInSegment();
		}
	}
	//unless it is handled previously;
	//1. update current position of vehicle/driver with xf
	//2. update current time with tf
	//3.vehicle->moveFwd();
//	vehicle->setPositionInSegment(xf);
//	p.timeThisTick = tf;

//	std::cout<< "end of advanceMoving - res:"<< (res? "True":"False") << std::endl;
	return res;
}

bool DriverMovement::advanceMovingVehicleWithInitialQ(DriverUpdateParams& p) {

	bool res = false;
	double t0 = p.elapsedSeconds;
	double x0 = vehicle->getPositionInSegment(); /*vehicle->getCurrSegment()->length - vehicle->getDistanceToSegmentStart();*/
	double xf = 0.0;
	double tf = 0.0;

	getSegSpeed();
	double vu = vehicle->getVelocity();

	//not implemented yet
	//double laneQueueLength = getQueueLength(p2.currLane);

	//not implemented
	double output = getOutputCounter(currLane);
	double outRate = getOutputFlowRate(currLane);

	double timeToDissipateQ = getInitialQueueLength(currLane)/(outRate*vehicle->length); //assuming vehicle length is in cm
	double timeToReachEndSeg = t0 + x0/vu;
	tf = std::max(timeToDissipateQ, timeToReachEndSeg);

	if (tf < p.secondsInTick)
	{
		if (output > 0)
		{
			res = moveToNextSegment(p);
		}
		else
		{
			addToQueue(currLane);
			//tf = p.secondsInTick; // melani 17-May
			p.elapsedSeconds = p.secondsInTick; //harish 20-May
		}
	}
	else
	{
		//cannot use == operator since it is double variable. tzl, Oct 18, 02
		if( fabs(tf-timeToReachEndSeg) < 0.001 && timeToReachEndSeg > p.secondsInTick)
		{
			tf = p.secondsInTick;
			xf = x0-vu*(tf-t0);
			res = moveInSegment(p, x0-xf);
			//p.currLaneOffset = vehicle->getDistanceMovedInSegment();
		}
		else
		{
			xf = 0.0 ;
			res = moveInSegment(p, x0-xf);
		}

		vehicle->setPositionInSegment(xf); //harish 20-May
		p.elapsedSeconds = tf; //harish 20-May
	}
	//1. update current position of vehicle/driver with xf
	//2. update current time with tf
//	std::cout<<"advanceMoving with initial Q rdSeg: "<<vehicle->getCurrSegment()->getStart()->getID()<<" setPos: "<< xf<<std::endl;

	//vehicle->setPositionInSegment(xf); commented by harish 20-May
	//p.elapsedSeconds = tf; commented harish 20-May
	return res;
}

void DriverMovement::getSegSpeed() {
	vehicle->setVelocity(vehicle->getCurrSegment()->
			getParentConflux()->getSegmentSpeed(vehicle->getCurrSegment(), true));
}

int DriverMovement::getOutputCounter(const Lane* l) {
	return parentAgent->getCurrSegment()->getParentConflux()->getOutputCounter(l);
}

double DriverMovement::getOutputFlowRate(const Lane* l) {
	return parentAgent->getCurrSegment()->getParentConflux()->getOutputFlowRate(l);
}

double DriverMovement::getAcceptRate(const Lane* l) {
	return parentAgent->getCurrSegment()->getParentConflux()->getAcceptRate(l);
}

double DriverMovement::getQueueLength(const Lane* l) {
	return ((parentAgent->getCurrSegment()->getParentConflux()->getLaneAgentCounts(l)).first) * (vehicle->length);
}

double DriverMovement::getLastAccept(const Lane* l) {
	return parentAgent->getCurrSegment()->getParentConflux()->getLastAccept(l);
}

void DriverMovement::setLastAccept(const Lane* l, double lastAccept) {
	parentAgent->getCurrSegment()->getParentConflux()->setLastAccept(l, lastAccept);
}

void DriverMovement::updateFlow(const RoadSegment* rdSeg, double startPos, double endPos) {
	double mid = rdSeg->computeLaneZeroLength()/2.0;
	if (startPos >= mid && mid >= endPos){
		rdSeg->getParentConflux()->incrementSegmentFlow(rdSeg);
	}
}

void DriverMovement::setOrigin(DriverUpdateParams& p) {

	//Vehicles start at rest
	vehicle->setVelocity(0);

	if(p.now.ms() < parentAgent->getStartTime()) {
		//set time to start - to accommodate drivers starting during the frame
		stepFwdInTime(p, (parentAgent->getStartTime() - p.now.ms())/1000.0);
	}

	const sim_mob::RoadSegment* nextRdSeg = nullptr;
	if (vehicle->hasNextSegment(true))
		nextRdSeg = vehicle->getNextSegment(true);

	else if (vehicle->hasNextSegment(false))
		nextRdSeg = vehicle->getNextSegment(false);

	nextLaneInNextSegment = getBestTargetLane(vehicle->getCurrSegment(), nextRdSeg);

	double departTime = getLastAccept(nextLaneInNextSegment) + getAcceptRate(nextLaneInNextSegment); //in seconds
	p.elapsedSeconds = std::max(p.elapsedSeconds, departTime - (p.now.ms()/1000.0));	//in seconds

	if(canGoToNextRdSeg(p, p.elapsedSeconds))
	{
		//set position to start
		if(vehicle->getCurrSegment())
		{
			vehicle->setPositionInSegment(vehicle->getCurrLinkLaneZeroLength());
		}
		currLane = nextLaneInNextSegment;
		double actualT = p.elapsedSeconds + (p.now.ms()/1000.0);
		Print() << "DriverMovement::setOrigin|Frame#: " << p.now.frame() << "|Person: " << parentAgent->getId()
				<< "|canGoToNextRdSeg successful|currLane set to: "<< currLane->getLaneID()
				<< "|Link: " << vehicle->getCurrSegment()->getLink()->getStart()->getID() << "->" << vehicle->getCurrSegment()->getLink()->getEnd()->getID()
				<< std::endl;
		parentAgent->initTravelStats(vehicle->getCurrSegment()->getLink(), actualT);

/*		std::cout<< parent->getId()<<" Driver is added at: "<< actualT*1000 << "ms to lane "<<
					nextLaneInNextSegment->getLaneID_str()
					<<" in RdSeg "<< (nextRdSeg? nextRdSeg->getStart()->getID() : 0)
					<<" last Accept: "<< getLastAccept(nextLaneInNextSegment)
					<<" accept rate: "<<getAcceptRate(nextLaneInNextSegment)
					<<"timeThisTick: "<<p.timeThisTick
					<<"now: "<<p.now.ms()
					<< std::endl;*/

		setLastAccept(currLane, actualT);
/*		std::cout<<"actualT: " <<actualT<<std::endl;*/
		setParentData(p);
		parentAgent->canMoveToNextSegment = Person::NONE;
	}
	else
	{
		p.elapsedSeconds = p.secondsInTick;
		setParentData(p);
		Print() << "DriverMovement::setOrigin|Frame#: " << p.now.frame() << "|Person: " << parentAgent->getId()
				<< "|canGoToNextRdSeg failed, will remain in lane infinity!" << std::endl;
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
	Person* parentP = dynamic_cast<Person*> (parentAgent);
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
	Person* parentP = dynamic_cast<Person*> (parentAgent);
	if (parentP) {
		if(parentP->isQueuing) {
			parentP->isQueuing = false;
			vehicle->isQueuing = false;
		}
		else {
			Print() << "removeFromQueue() was called for a driver who is not in queue. Person: " << parentP->getId()
					<< "|RoadSegment: " << currLane->getRoadSegment()->getStartEnd()
					<< "|Lane: " << currLane->getLaneID() << std::endl;
		}
	}
}

const sim_mob::Lane* DriverMovement::getBestTargetLane(const RoadSegment* nextRdSeg, const RoadSegment* nextToNextRdSeg) {
	//we have included getBastLG functionality here (get lane with minAllAgents)
	//before checking best lane
	//1. Get queueing counts for all lanes of the next Segment
	//2. Select the lane with the least queue length
	//3. Update nextLaneInNextLink and targetLaneIndex accordingly
	if(!nextRdSeg)
		return nullptr;

	const sim_mob::Lane* minQueueLengthLane = nullptr;
	const sim_mob::Lane* minAgentsLane = nullptr;
	unsigned int minQueueLength = std::numeric_limits<int>::max();
	unsigned int minAllAgents = std::numeric_limits<int>::max();
	unsigned int que = 0;
	unsigned int total = 0;
	int test_count = 0;

	vector<sim_mob::Lane* >::const_iterator i = nextRdSeg->getLanes().begin();

	//getBestLaneGroup logic
	for ( ; i != nextRdSeg->getLanes().end(); ++i){
		if ( !((*i)->is_pedestrian_lane())){
			if(nextToNextRdSeg) {
				if( !isConnectedToNextSeg(*i, nextToNextRdSeg))	continue;
			}
			que = vehicle->getCurrSegment()->getParentConflux()->getLaneAgentCounts(*i).first;
			total = que + vehicle->getCurrSegment()->getParentConflux()->getLaneAgentCounts(*i).second;

			if (minAllAgents > total){
				minAllAgents = total;
				minAgentsLane = *i;
			}
		}
	}

	//getBestLane logic
	for (i = nextRdSeg->getLanes().begin(); i != nextRdSeg->getLanes().end(); ++i){
		if ( !((*i)->is_pedestrian_lane())){
			if(nextToNextRdSeg) {
				if( !isConnectedToNextSeg(*i, nextToNextRdSeg)) continue;
			}
			que = vehicle->getCurrSegment()->getParentConflux()->getLaneAgentCounts(*i).first;
			total = que + vehicle->getCurrSegment()->getParentConflux()->getLaneAgentCounts(*i).second;
			if (minAllAgents == total){
				if (minQueueLength > que){
					minQueueLength = que;
					minQueueLengthLane = *i;
				}
			}
		}
	}

	if( !minQueueLengthLane){
		Print()<<"ERROR: best target lane was not set!" <<std::endl;
	}
	return minQueueLengthLane;
}

double DriverMovement::getInitialQueueLength(const Lane* l) {
	return parentAgent->getCurrSegment()->getParentConflux()->getInitialQueueCount(l) * vehicle->length;
}

void DriverMovement::insertIncident(const RoadSegment* rdSeg, double newFlowRate) {
	const vector<Lane*> lanes = rdSeg->getLanes();
	for (vector<Lane*>::const_iterator it = lanes.begin(); it != lanes.end(); it++) {
		rdSeg->getParentConflux()->updateLaneParams((*it), newFlowRate);
	}
}

void DriverMovement::frame_tick_output_mpi(timeslice now) {
	throw std::runtime_error("DriverMovement::frame_tick_output_mpi is not implemented yet");
}

void DriverMovement::removeIncident(const RoadSegment* rdSeg) {
	const vector<Lane*> lanes = rdSeg->getLanes();
	for (vector<Lane*>::const_iterator it = lanes.begin(); it != lanes.end(); it++){
		rdSeg->getParentConflux()->restoreLaneParams(*it);
	}
}

} /* namespace medium */
} /* namespace sim_mob */
