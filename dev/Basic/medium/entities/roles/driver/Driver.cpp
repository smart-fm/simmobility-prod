#include "Driver.hpp"

#include <cmath>
#include <ostream>
#include <algorithm>

#include "entities/Person.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/AuraManager.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"

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

#include "logging/Log.hpp"
#include "util/DebugFlags.hpp"

#include "partitions/PartitionManager.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"

using namespace sim_mob;

using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;

//Helper functions
namespace {
//TODO:I think lane index should be a data member in the lane class
size_t getLaneIndex(const Lane* l) {
	if (l) {
		const RoadSegment* r = l->getRoadSegment();
		for (size_t i = 0; i < r->getLanes().size(); i++) {
			if (r->getLanes().at(i) == l) {
				return i;
			}
		}
	}
	return -1; //NOTE: This might not do what you expect! ~Seth
}
} //end of anonymous namespace

//Initialize
sim_mob::medium::Driver::Driver(Agent* parent, MutexStrategy mtxStrat, sim_mob::medium::DriverBehavior* behavior, sim_mob::medium::DriverMovement* movement) :
	sim_mob::Role(behavior, movement, parent, "Driver_"), currLane(nullptr), vehicle(nullptr), nextLaneInNextSegment(nullptr), params(parent->getGenerator())
{

//	if (Debug::Drivers) {
//		DebugStream << "Driver starting: " << parent->getId() << endl;
//	}
}

///Note that Driver's destructor is only for reclaiming memory.
///  If you want to remove its registered properties from the Worker (which you should do!) then
///  this should occur elsewhere.
sim_mob::medium::Driver::~Driver() {
	//Our vehicle
	safe_delete_item(vehicle);
}

vector<BufferedBase*> sim_mob::medium::Driver::getSubscriptionParams() {
	vector<BufferedBase*> res;
	return res;
}

sim_mob::UpdateParams& sim_mob::medium::Driver::make_frame_tick_params(timeslice now)
{
	params.reset(now, *this);
	return params;
}

Role* sim_mob::medium::Driver::clone(Person* parent) const
{
	DriverBehavior* behavior = new DriverBehavior(parent);
	DriverMovement* movement = new DriverMovement(parent);
	Driver* driver = new Driver(parent, parent->getMutexStrategy(), behavior, movement);
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);
	return driver;
}

void sim_mob::medium::DriverUpdateParams::reset(timeslice now, const Driver& owner)
{
	UpdateParams::reset(now);

	//Reset; these will be set before they are used; the values here represent either default
	//       values or are unimportant.

	secondsInTick = ConfigParams::GetInstance().baseGranMS / 1000.0;

	elapsedSeconds = 0.0;

}



//void sim_mob::medium::Driver::setParentData() {
//	Person* parentP = dynamic_cast<Person*> (parent);
//	if(!vehicle->isDone()) {
//		if (parentP){
//		//	parentP->isQueuing = vehicle->isQueuing;
//			parentP->distanceToEndOfSegment = vehicle->getPositionInSegment();
//			parentP->movingVelocity = vehicle->getVelocity();
//		}
//		parent->setCurrLane(currLane);
//		parent->setCurrSegment(vehicle->getCurrSegment());
//	}
//	else {
//		if (parentP){
//			//	parentP->isQueuing = vehicle->isQueuing;
//			parentP->distanceToEndOfSegment = 0.0;
//			parentP->movingVelocity = 0.0;
//		}
//		parent->setCurrLane(nullptr);
//		parent->setCurrSegment(nullptr);
//	}
//}
//
//void sim_mob::medium::Driver::frame_tick_output(const UpdateParams& p)
//{
//	//Skip?
//	if (vehicle->isDone() || ConfigParams::GetInstance().using_MPI || ConfigParams::GetInstance().OutputDisabled()) {
//		return;
//	}
//
//	std::stringstream logout;
//	logout << "(\"Driver\""
//			<<","<<parent->getId()
//			<<","<<p.now.frame()
//			<<",{"
//			<<"\"RoadSegment\":\""<< (parent->getCurrSegment()->getSegmentID())
//			<<"\",\"Lane\":\""<<(parent->getCurrLane()->getLaneID())
//			<<"\",\"UpNode\":\""<<(parent->getCurrSegment()->getStart()->getID())
//			<<"\",\"DistanceToEndSeg\":\""<<parent->distanceToEndOfSegment;
//
//	if (this->parent->isQueuing) {
//			logout << "\",\"queuing\":\"" << "true";
//		} else {
//			logout << "\",\"queuing\":\"" << "false";
//		}
//
//		logout << "\"})" << std::endl;
//
//		LogOut(logout.str());
//}
//
//Vehicle* sim_mob::medium::Driver::initializePath(bool allocateVehicle) {
//	Vehicle* res = nullptr;
//
//	//Only initialize if the next path has not been planned for yet.
//	if(!parent->getNextPathPlanned()){
//		//Save local copies of the parent's origin/destination nodes.
//		origin.node = parent->originNode;
//		origin.point = origin.node->location;
//		goal.node = parent->destNode;
//		goal.point = goal.node->location;
//
//		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
//		vector<WayPoint> path;
//		Person* parentP = dynamic_cast<Person*> (parent);
//		if (!parentP || parentP->specialStr.empty()) {
//			const StreetDirectory& stdir = StreetDirectory::instance();
//			path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*origin.node), stdir.DrivingVertex(*goal.node));
//		}
//
//		//For now, empty paths aren't supported.
//		if (path.empty()) {
//			throw std::runtime_error("Can't initializePath(); path is empty.");
//		}
//
//		//TODO: Start in lane 0?
//		int startlaneID = 0;
//
//		// Bus should be at least 1200 to be displayed on Visualizer
//		const double length = 400;
//		const double width = 200;
//
//		//A non-null vehicle means we are moving.
//		if (allocateVehicle) {
//			res = new Vehicle(path, startlaneID, length, width);
//		}
//	}
//
//	//to indicate that the path to next activity is already planned
//	parent->setNextPathPlanned(true);
//	return res;
//}
//
//bool sim_mob::medium::Driver::moveToNextSegment(DriverUpdateParams& p)
//{
////1.	copy logic from DynaMIT::moveToNextSegment
////2.	add the corresponding changes from justLeftIntersection()
////3.	updateVelocity for the new segment
////4. 	vehicle->moveFwd()
//
//	bool res = false;
//
//	bool isNewLinkNext = ( !vehicle->hasNextSegment(true) && vehicle->hasNextSegment(false));
//
//	const sim_mob::RoadSegment* nextRdSeg = nullptr;
//
//	if (isNewLinkNext)
//		nextRdSeg = vehicle->getNextSegment(false);
//
//	else
//		nextRdSeg = vehicle->getNextSegment(true);
//
//	if ( !nextRdSeg) {
//		//vehicle is done
//		vehicle->actualMoveToNextSegmentAndUpdateDir_med();
//		if (vehicle->isDone()) {
//			parent->setToBeRemoved();
//		}
//		return false;
//	}
//
////	std::cout<<"Driver "<<parent->getId()<<" moveToNextSegment to "<< nextRdSeg->getStart()->getID()<<std::endl;
//
//	const sim_mob::RoadSegment* nextToNextRdSeg = vehicle->getSecondSegmentAhead();
//
//	nextLaneInNextSegment = getBestTargetLane(nextRdSeg, nextToNextRdSeg);
//
//	double departTime = getLastAccept(nextLaneInNextSegment) + getAcceptRate(nextLaneInNextSegment); //in seconds
//	p.elapsedSeconds = std::max(p.elapsedSeconds, departTime - (p.now.ms()/1000.0));	//in seconds
//
//	if (canGoToNextRdSeg(p, p.elapsedSeconds)){
//		if (vehicle->isQueuing){
//			removeFromQueue();
//		}
//		else{
//		//	removeFromMovingList();
//		}
//
//		currLane = nextLaneInNextSegment;
//		vehicle->actualMoveToNextSegmentAndUpdateDir_med();
//		vehicle->setPositionInSegment(vehicle->getCurrLinkLaneZeroLength());
//
//		double linkExitTimeSec =  p.elapsedSeconds + (p.now.ms()/1000.0);
//
//		if (isNewLinkNext)
//		{
//			//set Link Travel time for previous link
//			const RoadSegment* prevSeg = vehicle->getPrevSegment(false);
//			if (prevSeg){
//				const Link* prevLink = prevSeg->getLink();
//
//				//if prevLink is already in travelStats, update it's linkTT and add to travelStatsMap
//				if(prevLink == parent->getTravelStats().link_){
//					parent->addToTravelStatsMap(parent->getTravelStats(), linkExitTimeSec); //in seconds
//					prevSeg->getParentConflux()->setTravelTimes(parent, linkExitTimeSec);
//				}
//				//creating a new entry in agent's travelStats for the new link, with entry time
//				parent->initTravelStats(vehicle->getCurrSegment()->getLink(), linkExitTimeSec);
//			}
//
//		}
//
///*		std::cout<< parent->getId()<<" Driver is movedToNextSeg at: "<< linkExitTimeSec*1000 << "ms to lane "<<
//								currLane->getLaneID_str()
//								<<" in RdSeg "<< vehicle->getCurrSegment()->getStart()->getID()
//								<<" last Accept: "<< getLastAccept(currLane)
//								<<" accept rate: "<<getAcceptRate(currLane)
//								<<" timeThisTick: "<<p.timeThisTick
//								<<" now: "<<p.now.ms()
//								<< " dist2End: "<<vehicle->getPositionInSegment()
//								<< std::endl;*/
//
//		setLastAccept(currLane, linkExitTimeSec);
//		res = advance(p);
//	}
//
//	else{
////		std::cout<<"canGoTo failed!"<<std::endl;
//		if (vehicle->isQueuing){
//			moveInQueue();
//		}
//		else{
//			addToQueue(currLane);
//		}
//	}
//
//	return res;
//}
//
//bool sim_mob::medium::Driver::canGoToNextRdSeg(DriverUpdateParams& p, double t)
//{
//	//return false if the Driver cannot be added during this time tick
//	if (t >= p.secondsInTick) return false;
//
//	//check if the next road segment has sufficient empty space to accommodate one more vehicle
//	const RoadSegment* nextRdSeg = nextLaneInNextSegment->getRoadSegment();
//
//	if ( !nextRdSeg) return false;
//
//	unsigned int total = vehicle->getCurrSegment()->getParentConflux()->numMovingInSegment(nextRdSeg, true)
//			+ vehicle->getCurrSegment()->getParentConflux()->numQueueingInSegment(nextRdSeg, true);
//
//	int vehLaneCount = 0;
//
//	std::vector<sim_mob::Lane*>::const_iterator laneIt = nextRdSeg->getLanes().begin();
//	while(laneIt != nextRdSeg->getLanes().end())
//	{
//		if ( !(*laneIt)->is_pedestrian_lane())
//		{
//			vehLaneCount += 1;
//		}
//		laneIt++;
//	}
///*	std::cout << "nextRdSeg: "<<nextRdSeg->getStart()->getID()
//			<<" queueCount: " << vehicle->getCurrSegment()->getParentConflux()->numQueueingInSegment(nextRdSeg, true)
//			<<" movingCount: "<<vehicle->getCurrSegment()->getParentConflux()->numMovingInSegment(nextRdSeg, true)
//			<<" | numLanes: " << nextRdSeg->getLanes().size()
//			<<" | physical cap: " << vehLaneCount * nextRdSeg->computeLaneZeroLength()/vehicle->length - total
//			<<" | length: " << nextRdSeg->computeLaneZeroLength()
//			<< std::endl;*/
//
////	return total < (vehLaneCount * nextRdSeg->computeLaneZeroLength()/vehicle->length);
//	return vehicle->length <= (vehLaneCount * nextRdSeg->computeLaneZeroLength())
//			- (total*vehicle->length);
//}
//
//void sim_mob::medium::Driver::moveInQueue()
//{
//	//1.update position in queue (vehicle->setPosition(distInQueue))
//	//2.update p.timeThisTick
//	double positionOfLastUpdatedAgentInLane = 0.0;
//	if (parent->getCurrSegment()->getParentConflux()->getSegmentAgents().find(parent->getCurrSegment()) !=
//			parent->getCurrSegment()->getParentConflux()->getSegmentAgents().end()){
//		positionOfLastUpdatedAgentInLane = parent->getCurrSegment()->getParentConflux()->
//				getSegmentAgents()[parent->getCurrSegment()]->getPositionOfLastUpdatedAgentInLane(parent->getCurrLane());
//	}
//	if(positionOfLastUpdatedAgentInLane == -1.0)
//	{
//		vehicle->setPositionInSegment(0.0);
//	}
//	else
//	{
//		vehicle->setPositionInSegment(positionOfLastUpdatedAgentInLane +  vehicle->length);
//	}
//}
//
//const sim_mob::Lane* sim_mob::medium::Driver::getBestTargetLane(const RoadSegment* nextRdSeg, const RoadSegment* nextToNextRdSeg){
//	//we have included getBastLG functionality here (get lane with minAllAgents)
//	//before checking best lane
//	//1. Get queueing counts for all lanes of the next Segment
//	//2. Select the lane with the least queue length
//	//3. Update nextLaneInNextLink and targetLaneIndex accordingly
//	if(!nextRdSeg)
//		return nullptr;
//
//	const sim_mob::Lane* minQueueLengthLane = nullptr;
//	const sim_mob::Lane* minAgentsLane = nullptr;
//	unsigned int minQueueLength = std::numeric_limits<int>::max();
//	unsigned int minAllAgents = std::numeric_limits<int>::max();
//	unsigned int que = 0;
//	unsigned int total = 0;
//	int test_count = 0;
//
//	vector<sim_mob::Lane* >::const_iterator i = nextRdSeg->getLanes().begin();
//
//	//getBestLaneGroup logic
//	for ( ; i != nextRdSeg->getLanes().end(); ++i){
//		if ( !((*i)->is_pedestrian_lane())){
//			if(nextToNextRdSeg) {
//				if( !isConnectedToNextSeg(*i, nextToNextRdSeg))	continue;
//			}
//			que = vehicle->getCurrSegment()->getParentConflux()->getLaneAgentCounts(*i).first;
//			total = que + vehicle->getCurrSegment()->getParentConflux()->getLaneAgentCounts(*i).second;
//
//			if (minAllAgents > total){
//				minAllAgents = total;
//				minAgentsLane = *i;
//			}
//		}
//	}
//
//	//getBestLane logic
//	for (i = nextRdSeg->getLanes().begin(); i != nextRdSeg->getLanes().end(); ++i){
//		if ( !((*i)->is_pedestrian_lane())){
//			if(nextToNextRdSeg) {
//				if( !isConnectedToNextSeg(*i, nextToNextRdSeg)) continue;
//			}
//			que = vehicle->getCurrSegment()->getParentConflux()->getLaneAgentCounts(*i).first;
//			total = que + vehicle->getCurrSegment()->getParentConflux()->getLaneAgentCounts(*i).second;
//			if (minAllAgents == total){
//				if (minQueueLength > que){
//					minQueueLength = que;
//					minQueueLengthLane = *i;
//				}
//			}
//		}
//	}
//
//	if( !minQueueLengthLane){
//		Warn() <<"ERROR: best target lane was not set!" <<std::endl;
//	}
//	return minQueueLengthLane;
//}
//
//void sim_mob::medium::Driver::addToQueue(const Lane* lane) {
//	/* 1. set position to queue length in front
//	 * 2. set isQueuing = true
//	*/
//	vehicle->setPositionInSegment(getQueueLength(lane));
//
//	Person* parentP = dynamic_cast<Person*> (parent);
//	if (parentP) {
//		vehicle->isQueuing = true;
//		parentP->isQueuing = vehicle->isQueuing;
//	}
//}
//
//void sim_mob::medium::Driver::removeFromQueue() {
//
//	Person* parentP = dynamic_cast<Person*> (parent);
//	if (parentP) {
//		parentP->isQueuing = false;
//		vehicle->isQueuing = false;
//	}
//}
//
//bool sim_mob::medium::Driver::moveInSegment(DriverUpdateParams& p2, double distance)
//{
//	double startPos = vehicle->getPositionInSegment();
//
//	try {
//		vehicle->moveFwd_med(distance);
//	} catch (std::exception& ex) {
//		if (Debug::Drivers) {
//			if (ConfigParams::GetInstance().OutputEnabled()) {
//				DebugStream << ">>>Exception: " << ex.what() << endl;
//				PrintOut(DebugStream.str());
//			}
//		}
//
//		std::stringstream msg;
//		msg << "Error moving vehicle forward for Agent ID: " << parent->getId() << ","
//				<< this->vehicle->getPositionInSegment() << "\n" << ex.what();
//		throw std::runtime_error(msg.str().c_str());
//		return false;
//	}
//
//	double endPos = vehicle->getPositionInSegment();
//	updateFlow(vehicle->getCurrSegment(), startPos, endPos);
//
//	return true;
//}
//
//void sim_mob::medium::Driver::frame_tick(UpdateParams& p)
//{
//	DriverUpdateParams& p2 = dynamic_cast<DriverUpdateParams&>(p);
//
//	const Lane* laneInfinity = nullptr;
//	if(vehicle->getCurrSegment()->getParentConflux()->getSegmentAgents().find(vehicle->getCurrSegment())
//			!= vehicle->getCurrSegment()->getParentConflux()->getSegmentAgents().end()){
//		laneInfinity = vehicle->getCurrSegment()->getParentConflux()->
//			getSegmentAgents().at(vehicle->getCurrSegment())->laneInfinity;
//	}
//	if (vehicle && vehicle->hasPath() && laneInfinity !=nullptr) {
//		//at start vehicle will be in lane infinity. set origin will move it to the correct lane
//		if (parent->getCurrLane() == laneInfinity){ //for now
//			setOrigin(params);
//		}
//	} else {
//		Warn() <<"ERROR: Vehicle could not be created for driver; no route!" <<std::endl;
//	}
//
//	//Are we done already?
//	if (vehicle->isDone()) {
////		std::cout << parent->getId()<<" removed when frame_tick is called" << std::endl;
//		parent->setToBeRemoved();
//		return;
//	}
//
//	//======================================incident==========================================
///*
//	// this needs to be moved to be changed to read from input xml later
//	const sim_mob::RoadSegment* nextRdSeg = nullptr;
//	if (vehicle->hasNextSegment(true))
//		nextRdSeg = vehicle->getNextSegment(true);
//
//	else if (vehicle->hasNextSegment(false))
//		nextRdSeg = vehicle->getNextSegment(false);
//

//}
