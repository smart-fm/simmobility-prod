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

#include "util/OutputUtil.hpp"
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
sim_mob::medium::Driver::Driver(Agent* parent, MutexStrategy mtxStrat) :
	Role(parent, "Driver_"), /*remainingTimeToComplete(0),*/ currLane(nullptr), vehicle(nullptr),
	nextLaneInNextSegment(nullptr), params(parent->getGenerator())
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
	//res.push_back(&(currLane_));
	//res.push_back(&(currLaneOffset_));
	//res.push_back(&(currLaneLength_));
	return res;
}

Role* sim_mob::medium::Driver::clone(Person* parent) const
{
	Role* role = new Driver(parent, parent->getMutexStrategy());
	return role;
}

void sim_mob::medium::Driver::frame_init(UpdateParams& p)
{
	//Save the path from orign to next activity location in allRoadSegments
	if (!currResource) {
		Vehicle* veh = initializePath(true);
		if (veh) {
			safe_delete_item(vehicle);
			//To Do: Better to use currResource instead of vehicle, when handling other roles ~melani
			vehicle = veh;
			currResource = veh;
		}
	}
	else {
		initializePath(false);
	}
}

double sim_mob::medium::Driver::getTimeSpentInTick(DriverUpdateParams& p) {
	return p.timeThisTick;
}

void sim_mob::medium::Driver::stepFwdInTime(DriverUpdateParams& p,
		double time) {
	p.timeThisTick = p.timeThisTick + time;
}

void sim_mob::medium::Driver::setOrigin(DriverUpdateParams& p) {

	//Vehicles start at rest
	vehicle->setVelocity(0);

	//set time to start - to accommodate drivers starting during the frame
	stepFwdInTime(p, (parent->getStartTime() - p.now.ms())/1000.0);

	const sim_mob::RoadSegment* nextRdSeg = nullptr;
	if (vehicle->hasNextSegment(true))
		nextRdSeg = vehicle->getNextSegment(true);

	else if (vehicle->hasNextSegment(false))
		nextRdSeg = vehicle->getNextSegment(false);

	nextLaneInNextSegment = getBestTargetLane(vehicle->getCurrSegment(), nextRdSeg);

	double departTime = getLastAccept(nextLaneInNextSegment) + getAcceptRate(nextLaneInNextSegment); //in seconds
	p.timeThisTick = std::max(p.timeThisTick, departTime - (p.now.ms()/1000.0));	//in seconds

	if(canGoToNextRdSeg(p, p.timeThisTick))
	{
		//set position to start
		if(vehicle->getCurrSegment())
		{
			vehicle->setPositionInSegment(vehicle->getCurrLinkLaneZeroLength());
		}
		currLane = nextLaneInNextSegment;
		double actualT = p.timeThisTick + (p.now.ms()/1000.0);
		parent->initTravelStats(vehicle->getCurrSegment()->getLink(), actualT);

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
		setParentData();
	}
	else
	{
		/*std::cout<<"cangoto failed"<<std::endl;*/
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout << "Driver cannot be started in new segment, will remain in lane infinity!\n";
#endif
	}
}

sim_mob::UpdateParams& sim_mob::medium::Driver::make_frame_tick_params(timeslice now)
{
	params.reset(now, *this);
	return params;
}

void sim_mob::medium::DriverUpdateParams::reset(timeslice now, const Driver& owner)
{
	UpdateParams::reset(now);

	//Reset; these will be set before they are used; the values here represent either default
	//       values or are unimportant.

	elapsedSeconds = ConfigParams::GetInstance().baseGranMS / 1000.0;

	timeThisTick = 0.0;
}

void sim_mob::medium::Driver::setParentData() {
	Person* parentP = dynamic_cast<Person*> (parent);
	if(!vehicle->isDone()) {
		if (parentP){
		//	parentP->isQueuing = vehicle->isQueuing;
			parentP->distanceToEndOfSegment = vehicle->getPositionInSegment();
			parentP->movingVelocity = vehicle->getVelocity();
		}
		parent->setCurrLane(currLane);
		parent->setCurrSegment(vehicle->getCurrSegment());
	}
	else {
		if (parentP){
			//	parentP->isQueuing = vehicle->isQueuing;
			parentP->distanceToEndOfSegment = 0.0;
			parentP->movingVelocity = 0.0;
		}
		parent->setCurrLane(nullptr);
		parent->setCurrSegment(nullptr);
	}
}

void sim_mob::medium::Driver::frame_tick_output(const UpdateParams& p)
{
	//Skip?
	if (vehicle->isDone() || ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

#ifndef SIMMOB_DISABLE_OUTPUT
	std::stringstream logout;
	logout << "(\"Driver\""
			<<","<<parent->getId()
			<<","<<p.now.frame()
			<<",{"
			<<"\"RoadSegment\":\""<< (parent->getCurrSegment()->getSegmentID())
			<<"\",\"Lane\":\""<<(parent->getCurrLane()->getLaneID())
			<<"\",\"UpNode\":\""<<(parent->getCurrSegment()->getStart()->getID())
			<<"\",\"DistanceToEndSeg\":\""<<parent->distanceToEndOfSegment;

	if (this->parent->isQueuing) {
			logout << "\",\"queuing\":\"" << "true";
		} else {
			logout << "\",\"queuing\":\"" << "false";
		}

		logout << "\"})" << std::endl;

		LogOut(logout.str());
#endif
}

Vehicle* sim_mob::medium::Driver::initializePath(bool allocateVehicle) {
	Vehicle* res = nullptr;

	//Only initialize if the next path has not been planned for yet.
	if(!parent->getNextPathPlanned()){
		//Save local copies of the parent's origin/destination nodes.
		origin.node = parent->originNode;
		origin.point = origin.node->location;
		goal.node = parent->destNode;
		goal.point = goal.node->location;

		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
		vector<WayPoint> path;
		Person* parentP = dynamic_cast<Person*> (parent);
		if (!parentP || parentP->specialStr.empty()) {
			path = StreetDirectory::instance().SearchShortestDrivingPath(*origin.node, *goal.node);
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
	parent->setNextPathPlanned(true);
	return res;
}

bool sim_mob::medium::Driver::moveToNextSegment(DriverUpdateParams& p)
{
//1.	copy logic from DynaMIT::moveToNextSegment
//2.	add the corresponding changes from justLeftIntersection()
//3.	updateVelocity for the new segment
//4. 	vehicle->moveFwd()

	bool res = false;

	bool isNewLinkNext = ( !vehicle->hasNextSegment(true) && vehicle->hasNextSegment(false));

	const sim_mob::RoadSegment* nextRdSeg = nullptr;

	if (isNewLinkNext)
		nextRdSeg = vehicle->getNextSegment(false);

	else
		nextRdSeg = vehicle->getNextSegment(true);

	if ( !nextRdSeg) {
		//vehicle is done
		vehicle->actualMoveToNextSegmentAndUpdateDir_med();
		if (vehicle->isDone()) {
			parent->setToBeRemoved();
		}
		return false;
	}

//	std::cout<<"Driver "<<parent->getId()<<" moveToNextSegment to "<< nextRdSeg->getStart()->getID()<<std::endl;

	const sim_mob::RoadSegment* nextToNextRdSeg = vehicle->getSecondSegmentAhead();

	nextLaneInNextSegment = getBestTargetLane(nextRdSeg, nextToNextRdSeg);

	double departTime = getLastAccept(nextLaneInNextSegment) + getAcceptRate(nextLaneInNextSegment); //in seconds
	p.timeThisTick = std::max(p.timeThisTick, departTime - (p.now.ms()/1000.0));	//in seconds

	if (canGoToNextRdSeg(p, p.timeThisTick)){
		if (vehicle->isQueuing){
			removeFromQueue();
		}
		else{
		//	removeFromMovingList();
		}

		currLane = nextLaneInNextSegment;
		vehicle->actualMoveToNextSegmentAndUpdateDir_med();
		vehicle->setPositionInSegment(vehicle->getCurrLinkLaneZeroLength());

		double linkExitTimeSec =  p.timeThisTick + (p.now.ms()/1000.0);

		if (isNewLinkNext)
		{
			//set Link Travel time for previous link
			const RoadSegment* prevSeg = vehicle->getPrevSegment(false);
			if (prevSeg){
				const Link* prevLink = prevSeg->getLink();
/*				std::cout<<"prevLink: "<<prevLink->getStart()->getID()<<" | link_: "
						<<parent->getTravelStats().link_->getStart()->getID()<<
						" : "<<parent->getCurrSegment()->getLink()->getStart()->getID()
						<<std::endl;*/
				if(prevLink == parent->getTravelStats().link_){
					parent->addToTravelStatsMap(parent->getTravelStats(), linkExitTimeSec); //in seconds
					prevSeg->getParentConflux()->setTravelTimes(parent, linkExitTimeSec);
				}
				parent->initTravelStats(parent->getCurrSegment()->getLink(), linkExitTimeSec);
			}

		}

/*		std::cout<< parent->getId()<<" Driver is movedToNextSeg at: "<< linkExitTimeSec*1000 << "ms to lane "<<
								currLane->getLaneID_str()
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
//		std::cout<<"canGoTo failed!"<<std::endl;
		if (vehicle->isQueuing){
			moveInQueue();
		}
		else{
			addToQueue(currLane);
		}
	}

	return res;
}

bool sim_mob::medium::Driver::canGoToNextRdSeg(DriverUpdateParams& p, double t)
{
	if (t >= p.elapsedSeconds) return false;

	const RoadSegment* nextRdSeg = nextLaneInNextSegment->getRoadSegment();

	if ( !nextRdSeg) return false;

	unsigned int total = vehicle->getCurrSegment()->getParentConflux()->numMovingInSegment(nextRdSeg, true)
			+ vehicle->getCurrSegment()->getParentConflux()->numQueueingInSegment(nextRdSeg, true);

	int vehLaneCount = 0;

	std::vector<sim_mob::Lane*>::const_iterator laneIt = nextRdSeg->getLanes().begin();
	while(laneIt != nextRdSeg->getLanes().end())
	{
		if ( !(*laneIt)->is_pedestrian_lane())
		{
			vehLaneCount += 1;
		}
		laneIt++;
	}
/*	std::cout << "nextRdSeg: "<<nextRdSeg->getStart()->getID()
			<<" queueCount: " << vehicle->getCurrSegment()->getParentConflux()->numQueueingInSegment(nextRdSeg, true)
			<<" movingCount: "<<vehicle->getCurrSegment()->getParentConflux()->numMovingInSegment(nextRdSeg, true)
			<<" | numLanes: " << nextRdSeg->getLanes().size()
			<<" | physical cap: " << vehLaneCount * nextRdSeg->computeLaneZeroLength()/vehicle->length - total
			<<" | length: " << nextRdSeg->computeLaneZeroLength()
			<< std::endl;*/

//	return total < (vehLaneCount * nextRdSeg->computeLaneZeroLength()/vehicle->length);
	return vehicle->length <= (vehLaneCount * nextRdSeg->computeLaneZeroLength())
			- (total*vehicle->length);
}

void sim_mob::medium::Driver::moveInQueue()
{
	//1.update position in queue (vehicle->setPosition(distInQueue))
	//2.update p.timeThisTick
	double positionOfLastUpdatedAgentInLane = 0.0;
	if (parent->getCurrSegment()->getParentConflux()->getSegmentAgents().find(parent->getCurrSegment()) !=
			parent->getCurrSegment()->getParentConflux()->getSegmentAgents().end()){
		positionOfLastUpdatedAgentInLane = parent->getCurrSegment()->getParentConflux()->
				getSegmentAgents()[parent->getCurrSegment()]->getPositionOfLastUpdatedAgentInLane(parent->getCurrLane());
	}
	if(positionOfLastUpdatedAgentInLane == -1.0)
	{
		vehicle->setPositionInSegment(0.0);
	}
	else
	{
		vehicle->setPositionInSegment(positionOfLastUpdatedAgentInLane +  vehicle->length);
	}
}

const sim_mob::Lane* sim_mob::medium::Driver::getBestTargetLane(const RoadSegment* nextRdSeg, const RoadSegment* nextToNextRdSeg){
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
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout << "ERROR: best target lane was not set!\n";
#endif
	}
	return minQueueLengthLane;
}

void sim_mob::medium::Driver::addToQueue(const Lane* lane) {
	/* 1. set position to queue length in front
	 * 2. set isQueuing = true
	*/
	vehicle->setPositionInSegment(getQueueLength(lane));

	Person* parentP = dynamic_cast<Person*> (parent);
	if (parentP) {
		vehicle->isQueuing = true;
		parentP->isQueuing = vehicle->isQueuing;
	}
}

void sim_mob::medium::Driver::removeFromQueue() {

	Person* parentP = dynamic_cast<Person*> (parent);
	if (parentP) {
		parentP->isQueuing = false;
		vehicle->isQueuing = false;
	}
}

bool sim_mob::medium::Driver::moveInSegment(DriverUpdateParams& p2, double distance)
{
	try {
		vehicle->moveFwd_med(distance);
	} catch (std::exception& ex) {
		if (Debug::Drivers) {

#ifndef SIMMOB_DISABLE_OUTPUT
			DebugStream << ">>>Exception: " << ex.what() << endl;
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			std::cout << DebugStream.str();
#endif
		}

		std::stringstream msg;
		msg << "Error moving vehicle forward for Agent ID: " << parent->getId() << ","
				<< this->vehicle->getX() << "," << this->vehicle->getY() << "\n" << ex.what();
		throw std::runtime_error(msg.str().c_str());
		return false;
	}
	return true;
}

void sim_mob::medium::Driver::frame_tick(UpdateParams& p)
{
	DriverUpdateParams& p2 = dynamic_cast<DriverUpdateParams&>(p);

	const Lane* laneInfinity = nullptr;
	if(vehicle->getCurrSegment()->getParentConflux()->getSegmentAgents().find(vehicle->getCurrSegment())
			!= vehicle->getCurrSegment()->getParentConflux()->getSegmentAgents().end()){
		laneInfinity = vehicle->getCurrSegment()->getParentConflux()->
			getSegmentAgents().at(vehicle->getCurrSegment())->laneInfinity;
	}
	if (vehicle && vehicle->hasPath() && laneInfinity !=nullptr) {
		//at start vehicle will be in lane infinity. set origin will move it to the correct lane
		if (parent->getCurrLane() == laneInfinity){ //for now
			setOrigin(params);
		}
	} else {
		LogOut("ERROR: Vehicle could not be created for driver; no route!" <<std::endl);
	}

	//Are we done already?
	if (vehicle->isDone()) {
//		std::cout << parent->getId()<<" removed when frame_tick is called" << std::endl;
		parent->setToBeRemoved();
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

	//if vehicle is still in lane infinity, it shouldn't be advanced
	if (parent->getCurrLane() != laneInfinity)
	{
		advance(p2);
		//Update parent data. Only works if we're not "done" for a bad reason.
		setParentData();

	}
	else{
		std::cout << "lane or vehicle is not set for driver " <<parent->getId() << std::endl;
	}
}

bool sim_mob::medium::Driver::advance(DriverUpdateParams& p){
	if (vehicle->isDone()) {
		parent->setToBeRemoved();
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

bool sim_mob::medium::Driver::advanceQueuingVehicle(DriverUpdateParams& p){

	bool res = false;

	double t0 = p.timeThisTick;
	double x0 = vehicle->getPositionInSegment();
	double xf = 0.0;
	double tf = 0.0;

	double output = getOutputCounter(currLane);
	double outRate = getOutputFlowRate(currLane);
	tf = t0 + x0/(vehicle->length*outRate); //assuming vehicle length is in cm
	if (output > 0 && tf < p.elapsedSeconds)
	{
		res = moveToNextSegment(p);
		xf = vehicle->getPositionInSegment();
	}
	else
	{
		moveInQueue();
		xf = vehicle->getPositionInSegment();
		tf = p.elapsedSeconds;
	}
	//unless it is handled previously;
	//1. update current position of vehicle/driver with xf
	//2. update current time, p.timeThisTick, with tf
	vehicle->setPositionInSegment(xf);
//	std::cout<<"advanceQueuingVehicle rdSeg: "<<vehicle->getCurrSegment()->getStart()->getID()<<" setPos: "<< xf<<std::endl;
	p.timeThisTick = tf;

	return res;
}

bool sim_mob::medium::Driver::advanceMovingVehicle(DriverUpdateParams& p){

	bool res = false;
	double t0 = p.timeThisTick;
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
		p.timeThisTick = p.elapsedSeconds;
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

		if (tf < p.elapsedSeconds)
		{
			addToQueue(currLane);
			p.timeThisTick = p.elapsedSeconds;
		}
		else
		{
			xf = x0 - vu * (p.elapsedSeconds - t0);
			res = moveInSegment(p, x0 - xf);
//			std::cout<<"advanceMoving (with Q) rdSeg: "<<vehicle->getCurrSegment()->getStart()->getID()<<" setPos: "<< xf<<std::endl;
			vehicle->setPositionInSegment(xf);
			p.timeThisTick = p.elapsedSeconds;
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
		if (tf < p.elapsedSeconds)
		{
/*			ss << vehicle->getCurrSegment()->getStart()->getID()
					<<"tf less than tick | output: " << output << endl;
			std::cout<<ss.str();
			ss.str("");*/

			if (output > 0)
			{
				p.timeThisTick = tf;
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
				p.timeThisTick = p.elapsedSeconds;
			}
		}
		else
		{
//			std::cout << "tf more than tick" << std::endl;
			tf = p.elapsedSeconds;
			xf = x0-vu*(tf-t0);
			res = moveInSegment(p, x0-xf);
			vehicle->setPositionInSegment(xf);
//			std::cout<<"advanceMovingVehicle(no Q) rdSeg: "<<vehicle->getCurrSegment()->getStart()->getID()<<" setPos: "<<xf<<std::endl;

/*			std::cout<<"new position in advance: "<< vehicle->getPositionInSegment()
					<<" for segment "<<vehicle->getCurrSegment()->getStart()->getID()
					<<" veh lane: "<<vehicle->getCurrLane()->getLaneID_str()
					<<" lane: "<<currLane->getLaneID_str()
					<<std::endl;*/
			p.timeThisTick = tf;
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

bool sim_mob::medium::Driver::advanceMovingVehicleWithInitialQ(DriverUpdateParams& p){

	bool res = false;
	double t0 = p.timeThisTick;
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

	if (tf < p.elapsedSeconds)
	{
		if (output > 0)
		{
			res = moveToNextSegment(p);
		}
		else
		{
			addToQueue(currLane);
		}
	}
	else
	{
		//cannot use == operator since it is double variable. tzl, Oct 18, 02
		if( fabs(tf-timeToReachEndSeg) < 0.001 && timeToReachEndSeg > p.elapsedSeconds)
		{
			tf = p.elapsedSeconds;
			xf = x0-vu*(tf-t0);
			res = moveInSegment(p, x0-xf);
			//p.currLaneOffset = vehicle->getDistanceMovedInSegment();
		}
		else
		{
			xf = 0.0 ;
			res = moveInSegment(p, x0-xf);
		}
	}
	//1. update current position of vehicle/driver with xf
	//2. update current time with tf
//	std::cout<<"advanceMoving with initial Q rdSeg: "<<vehicle->getCurrSegment()->getStart()->getID()<<" setPos: "<< xf<<std::endl;
	vehicle->setPositionInSegment(xf);
	p.timeThisTick = tf;

	return res;
}

void sim_mob::medium::Driver::getSegSpeed(){

	vehicle->setVelocity(vehicle->getCurrSegment()->
			getParentConflux()->getSegmentSpeed(vehicle->getCurrSegment(), true));
}

int sim_mob::medium::Driver::getOutputCounter(const Lane* l) {
	return parent->getCurrSegment()->getParentConflux()->getOutputCounter(l);
}

double sim_mob::medium::Driver::getOutputFlowRate(const Lane* l) {
	return parent->getCurrSegment()->getParentConflux()->getOutputFlowRate(l);
}

double sim_mob::medium::Driver::getAcceptRate(const Lane* l) {
	return parent->getCurrSegment()->getParentConflux()->getAcceptRate(l);
}

double sim_mob::medium::Driver::getQueueLength(const Lane* l) {
	return ((parent->getCurrSegment()->getParentConflux()->getLaneAgentCounts(l)).first) * (vehicle->length);
}

bool sim_mob::medium::Driver::isConnectedToNextSeg(const Lane* lane, const RoadSegment* nextRdSeg){
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

double sim_mob::medium::Driver::getInitialQueueLength(const Lane* l) {
	return parent->getCurrSegment()->getParentConflux()->getInitialQueueCount(l) * vehicle->length;
}

double sim_mob::medium::Driver::getLastAccept(const Lane* l){
	return parent->getCurrSegment()->getParentConflux()->getLastAccept(l);
}

void sim_mob::medium::Driver::setLastAccept(const Lane* l, double lastAccept){
	parent->getCurrSegment()->getParentConflux()->setLastAccept(l, lastAccept);
}

void sim_mob::medium::Driver::insertIncident(const RoadSegment* rdSeg, double newFlowRate){
	const vector<Lane*> lanes = rdSeg->getLanes();
	for (vector<Lane*>::const_iterator it = lanes.begin(); it != lanes.end(); it++) {
		rdSeg->getParentConflux()->getSegmentAgents()[rdSeg]->updateLaneParams((*it), newFlowRate);
	}
}

void sim_mob::medium::Driver::removeIncident(const RoadSegment* rdSeg){
	const vector<Lane*> lanes = rdSeg->getLanes();
	for (vector<Lane*>::const_iterator it = lanes.begin(); it != lanes.end(); it++){
		rdSeg->getParentConflux()->getSegmentAgents()[rdSeg]->restoreLaneParams(*it);
	}
}
