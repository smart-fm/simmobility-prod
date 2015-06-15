//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverFacets.hpp"
#include "MesoReroute.hpp"
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
#include "path/PathSetManager.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "path/PathSetManager.hpp"
#include "message/MessageBus.hpp"

#include "logging/Log.hpp"

#include "partitions/PartitionManager.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"

#include "path/ScreenLineCounter.hpp"

#include "util/DebugFlags.hpp"
#include "util/Utils.hpp"

#include "boost/foreach.hpp"
using namespace sim_mob;
using namespace sim_mob::medium;
using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;

namespace{
sim_mob::BasicLogger & pathsetLogger = sim_mob::Logger::log("pathset.log");
}

namespace {
/**
 * converts time from milli-seconds to seconds
 */
inline double convertToSeconds(uint32_t timeInMs) {
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
	isQueuing(false), laneConnectorOverride(false)
{
	rerouter.reset(new MesoReroute(*this));
}


sim_mob::medium::DriverMovement::~DriverMovement() {
	/*
	 * possible candidate place for finalize
	 * if(!travelMetric.finalized){
		finalizeTravelTimeMetric();
	}*/
}

void sim_mob::medium::DriverMovement::frame_init() {
	bool pathInitialized = initializePath();
	if (pathInitialized) {
		Vehicle* newVehicle = new Vehicle(Vehicle::CAR, PASSENGER_CAR_UNIT);
		VehicleBase* oldVehicle = parentDriver->getResource();
		safe_delete_item(oldVehicle);
		parentDriver->setResource(newVehicle);
	}
	else{ getParent()->setToBeRemoved(); }
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
			getParent()->canMoveToNextSegment = Person::NONE;
			setParentData(params);
			return;
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
			<<"\"RoadSegment\":\""<< (getParent()->getCurrSegStats()->getRoadSegment()->getId())
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

void sim_mob::medium::DriverMovement::randomizeStartingSegment(std::vector<sim_mob::WayPoint>& wpPath)
{
	if(wpPath.size() < 2) { return; } //no randomization for very short paths

	//compute number of segments in the first link of path
	int numSegsInFirstLink = 0;
	sim_mob::Node* firstLinkEnd = nullptr;
	for (vector<sim_mob::WayPoint>::const_iterator it = wpPath.begin(); it != wpPath.end(); it++)
	{
		if (it->type_ == WayPoint::ROAD_SEGMENT)
		{
			const sim_mob::RoadSegment* rdSeg = it->roadSegment_;
			if(!firstLinkEnd) {	firstLinkEnd = rdSeg->getLink()->getEnd(); }
			numSegsInFirstLink++;
			if(firstLinkEnd == rdSeg->getEnd()) { break; }
		}
	}

	if(numSegsInFirstLink >= wpPath.size()) { return; } //no randomization if the entire path is contained in 1 link

	//generate uniform random number between 0 and numSegsInFirstLink-1 (minus 1 to keep atleast 1 segment in first link)
	int randomIdx = Utils::generateInt(0, numSegsInFirstLink-1);
	//remove that many elements from the front of the path
	//the removals are guaranteed to stay within the first link
	for(int i=0; i<randomIdx; i++)
	{
		if (wpPath.front().type_ != WayPoint::ROAD_SEGMENT) { wpPath.erase(wpPath.begin()); } //extra erase for other items which are not Road segments
		wpPath.erase(wpPath.begin());
	}
}

void sim_mob::medium::DriverMovement::initSegStatsPath(vector<sim_mob::WayPoint>& wpPath, vector<const sim_mob::SegmentStats*>& ssPath) {
	for (vector<sim_mob::WayPoint>::iterator it = wpPath.begin(); it != wpPath.end(); it++)
	{
		if (it->type_ == WayPoint::ROAD_SEGMENT)
		{
			const sim_mob::RoadSegment* rdSeg = it->roadSegment_;
			const vector<sim_mob::SegmentStats*>& statsInSegment = rdSeg->getParentConflux()->findSegStats(rdSeg);
			ssPath.insert(ssPath.end(), statsInSegment.begin(), statsInSegment.end());
		}
	}
}

void sim_mob::medium::DriverMovement::initSegStatsPath(const std::vector<const sim_mob::RoadSegment*>& rsPath, std::vector<const sim_mob::SegmentStats*>& ssPath)
{
	for (vector<const sim_mob::RoadSegment*>::const_iterator it = rsPath.begin(); it != rsPath.end(); it++)
	{
		const sim_mob::RoadSegment* rdSeg = *it;
		const vector<sim_mob::SegmentStats*>& statsInSegment = rdSeg->getParentConflux()->findSegStats(rdSeg);
		ssPath.insert(ssPath.end(), statsInSegment.begin(), statsInSegment.end());
	}
}

bool sim_mob::medium::DriverMovement::initializePath()
{
	//Only initialize if the next path has not been planned for yet.
	sim_mob::Person* person = getParent();
	if(!person->getNextPathPlanned())
	{
		//Save local copies of the parent's origin/destination nodes.
		parentDriver->origin.node = person->originNode.node_;
		parentDriver->origin.point = parentDriver->origin.node->location;
		parentDriver->goal.node = person->destNode.node_;
		parentDriver->goal.point = parentDriver->goal.node->location;

		if(person->originNode.node_ == person->destNode.node_)
		{
			Print() << "DriverMovement::initializePath | Can't initializePath because origin and destination are the same for driver " << person->GetId()
			<< "\norigin:" << person->originNode.node_->getID() << "\ndestination:" << person->destNode.node_->getID() << std::endl;
			return false;
		}

		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
		vector<WayPoint> wp_path;
		const sim_mob::SubTrip& currSubTrip = *(person->currSubTrip);
		if (ConfigManager::GetInstance().FullConfig().PathSetMode()) // if use path set
		{
			wp_path = PathSetManager::getInstance()->getPath(currSubTrip, false, nullptr);
		}
		else
		{
			const StreetDirectory& stdir = StreetDirectory::instance();
			wp_path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*(parentDriver->origin).node), stdir.DrivingVertex(*(parentDriver->goal).node));
		}

		randomizeStartingSegment(wp_path); //start driver in random segment of first link

		if (wp_path.empty()) //ideally should not be empty after randomization.
		{
			Print()<<"Can't DriverMovement::initializePath(); path is empty for driver "  << person->GetId() << std::endl;
			return false;
		}

		//Restricted area logic
		if (sim_mob::ConfigManager::GetInstance().FullConfig().CBD())
		{
			bool fromLocationInRestrictedRegion = sim_mob::RestrictedRegion::getInstance().isInRestrictedZone(wp_path.front().roadSegment_->getStart());
			bool toLocationInRestrictedRegion = sim_mob::RestrictedRegion::getInstance().isInRestrictedZone(wp_path.back().roadSegment_->getEnd());
			if (!toLocationInRestrictedRegion && !fromLocationInRestrictedRegion)
			{//both O & D outside
				if (sim_mob::RestrictedRegion::getInstance().isInRestrictedSegmentZone(wp_path))
				{
					currSubTrip.cbdTraverseType = TravelMetric::CBD_PASS;
				}
			}
			else if(!(toLocationInRestrictedRegion && fromLocationInRestrictedRegion))
			{//exactly one of O & D is inside restricted region
				currSubTrip.cbdTraverseType = fromLocationInRestrictedRegion ? TravelMetric::CBD_EXIT : TravelMetric::CBD_ENTER;
			}
			//else we leave the cbdTraverseType as CBD_NONE
		}

		std::vector<const sim_mob::SegmentStats*> path;
		initSegStatsPath(wp_path, path);
		if(path.empty()) { return false; }
		pathMover.setPath(path);
		const sim_mob::SegmentStats* firstSegStat = path.front();
		person->setCurrSegStats(firstSegStat);
		person->setCurrLane(firstSegStat->laneInfinity);
		person->distanceToEndOfSegment = firstSegStat->getLength();
	}
	person->setNextPathPlanned(true); //to indicate that the path to next activity is already planned
	return true;
}

void DriverMovement::setParentData(sim_mob::medium::DriverUpdateParams& params) {
	if(!pathMover.isPathCompleted()) {
		parent->distanceToEndOfSegment = pathMover.getPositionInSegment();
		parent->setCurrLane(currLane);
		parent->setCurrSegStats(pathMover.getCurrSegStats());
		parent->setRemainingTimeThisTick(params.secondsInTick - params.elapsedSeconds);
	}
	else {
		parent->distanceToEndOfSegment = 0.0;
		parent->setCurrLane(nullptr);
		parent->setCurrSegStats(nullptr);
		parent->setRemainingTimeThisTick(0.0);
		parent->isQueuing = false;
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

	//currently the best place to call a handler indicating 'Done' with segment.
	const sim_mob::RoadSegment *curRs = (*(pathMover.getCurrSegStats())).getRoadSegment();
	//Although the name of the method suggests segment change, it is actually segStat change. so we check again!
	const sim_mob::RoadSegment *nxtRs = (nxtSegStat ? nxtSegStat->getRoadSegment() : nullptr);

	if(curRs && nxtSegStat && curRs != nxtRs)
	{
		onSegmentCompleted(curRs,nxtRs);
	}

	if(isNewLinkNext)
	{
		onLinkCompleted(curRs->getLink(),(nxtRs ? nxtRs->getLink() : nullptr));
	}

	//reset these local variables in case path has been changed in onLinkCompleted
	isNewLinkNext = (!pathMover.hasNextSegStats(true) && pathMover.hasNextSegStats(false));
	currSegStat = pathMover.getCurrSegStats();
	nxtSegStat = pathMover.getNextSegStats(!isNewLinkNext);


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
	const Lane* laneInNextSegment = getBestTargetLane(nxtSegStat, nextToNextSegStat);

	//this will space out the drivers on the same lane, by seperating them by the time taken for the previous car to move a car's length
	//Commenting out the delay from accept rate as per Yang Lu's suggestion (we only use this delay in setOrigin)
	double departTime = getLastAccept(laneInNextSegment, nxtSegStat)
			 /*+ getAcceptRate(laneInNextSegment, nxtSegStat)*/; //in seconds

	//skip acceptance capacity if there's no queue - this is done in DynaMIT
	//commenting out - the delay from acceptRate is removed as per Yang Lu's suggestion
/*	if(nextRdSeg->getParentConflux()->numQueueingInSegment(nextRdSeg, true) == 0){
		departTime = getLastAccept(nextLaneInNextSegment)
						+ (0.01 * vehicle->length) / (nextRdSeg->getParentConflux()->getSegmentSpeed(nextRdSeg, true) ); // skip input capacity
	}*/

	params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - convertToSeconds(params.now.ms())); //in seconds

	const Link* nextLink = getNextLinkForLaneChoice(nxtSegStat);
	if (canGoToNextRdSeg(params, nxtSegStat, nextLink)){
		if (isQueuing){
			removeFromQueue();
		}

		setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat)-1), currSegStat); // decrement from the currLane before updating it
		currLane = laneInNextSegment;
		pathMover.advanceInPath();
		pathMover.setPositionInSegment(nxtSegStat->getLength());
		double segExitTimeSec =  params.elapsedSeconds + (convertToSeconds(params.now.ms()));
		setLastAccept(currLane, segExitTimeSec, nxtSegStat);

		const sim_mob::SegmentStats* prevSegStats = pathMover.getPrevSegStats(true);	//previous segment is in the same link
		if(prevSegStats){
			// update road segment travel times
			updateRdSegTravelTimes(prevSegStats, segExitTimeSec);
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

/*
 * List of the operations performed in this method
 * 1- CBD Travel Metrics collections
 * 2- Rerouting
 */
void DriverMovement::onSegmentCompleted(const sim_mob::RoadSegment* completedRS, const sim_mob::RoadSegment* nextRS)
{
	//1.record
	traversed.push_back(completedRS);

	//2. update travel distance
	travelMetric.distance += completedRS->getPolylineLength();

	//3. CBD
	processCBD_TravelMetrics(completedRS, nextRS);
}

void DriverMovement::onLinkCompleted(const sim_mob::Link * completedLink, const sim_mob::Link * nextLink)
{
	//2. Rerouting
	if(ConfigManager::GetInstance().FullConfig().pathSet().reroute)
	{
		reroute();
	}
}

void DriverMovement::flowIntoNextLinkIfPossible(sim_mob::medium::DriverUpdateParams& params) {
	//This function gets called for 2 cases.
	//1. Driver is added to virtual queue
	//2. Driver is in previous segment trying to add to the next
	const sim_mob::SegmentStats* currSegStat = pathMover.getCurrSegStats();
	const sim_mob::SegmentStats* nextSegStats = pathMover.getNextSegStats(false);
	const sim_mob::SegmentStats* nextToNextSegStats = pathMover.getSecondSegStatsAhead();
	const Lane* laneInNextSegment = getBestTargetLane(nextSegStats, nextToNextSegStats);

	//this will space out the drivers on the same lane, by seperating them by the time taken for the previous car to move a car's length
	//Commenting out the delay from accept rate as per Yang Lu's suggestion (we use this delay only in setOrigin)
	double departTime = getLastAccept(laneInNextSegment, nextSegStats) /*+ getAcceptRate(laneInNextSegment, nextSegStats)*/; //in seconds

	params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (convertToSeconds(params.now.ms()))); //in seconds

	const Link* nextLink = getNextLinkForLaneChoice(nextSegStats);
	if (canGoToNextRdSeg(params, nextSegStats, nextLink)) {
		if (isQueuing){
			removeFromQueue();
		}

		currLane = laneInNextSegment;
		pathMover.advanceInPath();
		pathMover.setPositionInSegment(nextSegStats->getLength());

		//todo: consider supplying milliseconds to be consistent with short-term
		double linkExitTimeSec =  params.elapsedSeconds + (convertToSeconds(params.now.ms()));
		//set Link Travel time for previous link
		const SegmentStats* prevSegStats = pathMover.getPrevSegStats(false);
		if (prevSegStats) {
			// update link travel times
			updateLinkTravelTimes(prevSegStats, linkExitTimeSec);

			// update road segment travel times
			updateRdSegTravelTimes(prevSegStats, linkExitTimeSec);
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
					<< "|id: " << pathMover.getCurrSegStats()->getRoadSegment()->getId()
					<< "|lane: " << currLane->getLaneID()
					<< "\nPerson| segment: " << getParent()->getCurrSegStats()->getRoadSegment()->getStartEnd()
					<< "|id: " << getParent()->getCurrSegStats()->getRoadSegment()->getId()
					<< "|lane: " << (getParent()->getCurrLane()? getParent()->getCurrLane()->getLaneID():0)
					<< std::endl;

			throw::std::runtime_error(DebugStream.str());
		}
		params.elapsedSeconds = params.secondsInTick;
		getParent()->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
	}
}

bool DriverMovement::canGoToNextRdSeg(sim_mob::medium::DriverUpdateParams& params, const sim_mob::SegmentStats* nextSegStats, const Link* nextLink) const
{
	//return false if the Driver cannot be added during this time tick
	if (params.elapsedSeconds >= params.secondsInTick) {
		return false;
	}

	//check if the next road segment has sufficient empty space to accommodate one more vehicle
	if (!nextSegStats) {
		return false;
	}

	double enteringVehicleLength =  parentDriver->getResource()->getLengthCm();
	double maxAllowed = nextSegStats->getNumVehicleLanes() * nextSegStats->getLength();
	double total = nextSegStats->getTotalVehicleLength();

	//if the segment is shorter than the vehicle's length and there are no vehicles in the segment just allow the vehicle to pass through
	//this is just an interim arrangement. this segment should either be removed from database or it's length must be updated.
	//if this hack is not in place, all vehicles will start queuing in upstream segments forever.
	//TODO: remove this hack and put permanent fix
	if((maxAllowed < enteringVehicleLength) && (total <= 0)) { return true; }

	//if this segment is a bus terminus segment, we assume only buses try to enter this segment and allow the bus inside irrespective of available space.
	if(nextSegStats->getRoadSegment()->isBusTerminusSegment()) { return true; }

	bool hasSpaceInNextStats = ((maxAllowed - total) >= enteringVehicleLength);
	if(hasSpaceInNextStats && nextLink)
	{
		//additionally check if the length of vehicles in the lanegroup is not too long to accommodate this driver
		double maxAllowedInLG = nextSegStats->getAllowedVehicleLengthForLaneGroup(nextLink);
		double totalInLG = nextSegStats->getVehicleLengthForLaneGroup(nextLink);
		return (totalInLG < maxAllowedInLG);
	}
	return hasSpaceInNextStats;
}

void DriverMovement::moveInQueue() {
	//1.update position in queue (vehicle->setPosition(distInQueue))
	//2.update p.timeThisTick
	double positionOfLastUpdatedAgentInLane = pathMover.getCurrSegStats()->getPositionOfLastUpdatedAgentInLane(currLane);
	if(positionOfLastUpdatedAgentInLane == -1.0)
	{
		pathMover.setPositionInSegment(0.0);
	}
	else
	{
		pathMover.setPositionInSegment(positionOfLastUpdatedAgentInLane	/*+ parentDriver->getResource()->getLengthCm()*/);
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

	if (output > 0 && finalTimeSpent < params.secondsInTick
			&& pathMover.getCurrSegStats()->getPositionOfLastUpdatedAgentInLane(currLane) == -1)
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

	const Lane* laneInNextSegment = getBestTargetLane(currSegStats, nextSegStats);

	//this will space out the drivers on the same lane, by separating them by the time taken for the previous car to move a car's length
	double departTime = getLastAccept(laneInNextSegment, currSegStats) + getAcceptRate(laneInNextSegment, currSegStats); //in seconds

	/*//skip acceptance capacity if there's no queue - this is done in DynaMIT
	if(getCurrSegment()->getParentConflux()->numQueueingInSegment(getCurrSegment(), true) == 0){
		departTime = getLastAccept(nextLaneInNextSegment)
						+ (0.01 * vehicle->length) / (getCurrSegment()->getParentConflux()->getSegmentSpeed(getCurrSegment(), true) ); // skip input capacity
	}*/

	params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (convertToSeconds(params.now.ms())));	//in seconds

	const Link* nextLink = getNextLinkForLaneChoice(currSegStats);
	if(canGoToNextRdSeg(params, currSegStats, nextLink))
	{
		//set position to start
		if(currSegStats)
		{
			pathMover.setPositionInSegment(currSegStats->getLength());
		}
		currLane = laneInNextSegment;
		double actualT = params.elapsedSeconds + (convertToSeconds(params.now.ms()));
		getParent()->initLinkTravelStats(currSegStats->getRoadSegment()->getLink(), actualT);

		setLastAccept(currLane, actualT, currSegStats);
		setParentData(params);
		getParent()->canMoveToNextSegment = Person::NONE;
		// segment travel time related line(s)
		getParent()->startCurrRdSegTravelStat(currSegStats->getRoadSegment(), actualT);

		if(getParentDriver()->roleType == sim_mob::Role::RL_DRIVER)
		{
			//initialize some travel metrics for this subTrip
			startTravelTimeMetric(); //not for bus drivers or any other role
		}
	}
	else
	{
		params.elapsedSeconds = params.secondsInTick;
		getParent()->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
	}
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

const sim_mob::Lane* DriverMovement::getBestTargetLane(const SegmentStats* nextSegStats, const SegmentStats* nextToNextSegStats)
{
	if(!nextSegStats) { return nullptr; }
	const sim_mob::Lane* minLane = nullptr;
	double minQueueLength = std::numeric_limits<double>::max();
	double minLength = std::numeric_limits<double>::max();
	double queueLength = 0.0;
	double totalLength = 0.0;

	const sim_mob::Link* nextLink = getNextLinkForLaneChoice(nextSegStats);
	const std::vector<sim_mob::Lane*>& lanes = nextSegStats->getRoadSegment()->getLanes();
	for (vector<sim_mob::Lane* >::const_iterator lnIt = lanes.begin(); lnIt != lanes.end(); ++lnIt)
	{
		const Lane* lane = *lnIt;
		if (!lane->is_pedestrian_lane() && !lane->is_whole_day_bus_lane())
		{
			if(!laneConnectorOverride
					&& nextToNextSegStats
					&& !isConnectedToNextSeg(lane, nextToNextSegStats->getRoadSegment())
					&& nextLink
					&& !nextSegStats->isConnectedToDownstreamLink(nextLink, lane))
			{ continue; }
			totalLength = nextSegStats->getLaneTotalVehicleLength(lane);
			queueLength = nextSegStats->getLaneQueueLength(lane);
			if (minLength > totalLength)
			{	//if total length of vehicles is less than current minLength
				minLength = totalLength;
				minQueueLength = queueLength;
				minLane = lane;
			}
			else if (minLength == totalLength)
			{	//if total length of vehicles is equal to current minLength
				if (minQueueLength > queueLength)
				{	//and if the queue length is less than current minQueueLength
					minQueueLength = queueLength;
					minLane = lane;
				}
			}
		}
	}

	if(!minLane)
	{
		Print() << "\nCurrent Path " << pathMover.getPath().size() << std::endl;
		Print() << MesoPathMover::printPath(pathMover.getPath());

		std::ostringstream out("");
		out << "best target lane was not set!" << "\nCurrent Segment: " << pathMover.getCurrSegStats()->getRoadSegment()->getSegmentAimsunId() <<
				" =>" << nextSegStats->getRoadSegment()->getSegmentAimsunId() <<
				" =>" <<  nextToNextSegStats->getRoadSegment()->getSegmentAimsunId()  << std::endl;
		out << "firstSegInNextLink:" <<  (nextLink? nextLink->getSegments().front()->getSegmentAimsunId() : 0)
				<< "|NextLink: " << (nextLink? nextLink->getLinkId() : 0)
				<< "|downstreamLinks of " << nextSegStats->getRoadSegment()->getSegmentAimsunId() << std::endl;

		Print() << out.str();
		nextSegStats->printDownstreamLinks();
		throw std::runtime_error(out.str());
	}
	return minLane;
}

double DriverMovement::getInitialQueueLength(const Lane* lane) {
	return pathMover.getCurrSegStats()->getInitialQueueLength(lane);
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

void DriverMovement::updateRdSegTravelTimes(const sim_mob::SegmentStats* prevSegStat, double segEnterExitTime) {
	//if prevSeg is already in travelStats, update it's rdSegTT and add to rdSegTravelStatsMap
	const RoadSegment* prevSeg= prevSegStat->getRoadSegment();
	sim_mob::Person *parent = getParent();
	if(prevSeg == parent->getCurrRdSegTravelStats().rs) {
		const sim_mob::TripChainItem* tripChain = *(parent->currTripChainItem);
		const std::string& travelMode = tripChain->getMode();

		sim_mob::Agent::RdSegTravelStat & currStats = parent->finalizeCurrRdSegTravelStat(prevSeg,segEnterExitTime, travelMode);

		if(ConfigManager::GetInstance().FullConfig().PathSetMode())
		{
			PathSetManager::getInstance()->addSegTT(currStats);
		}

		ScreenLineCounter::getInstance()->updateScreenLineCount(currStats);
	}
	//creating a new entry in agent's travelStats for the new road segment, with entry time
	parent->getCurrRdSegTravelStats().reset();
	parent->startCurrRdSegTravelStat(pathMover.getCurrSegStats()->getRoadSegment(), segEnterExitTime);
}

TravelMetric & sim_mob::medium::DriverMovement::startTravelTimeMetric()
{
	std::string now((DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime()).getStrRepr());
	travelMetric.startTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
	const Node* startNode = (*(pathMover.getPath().begin()))->getRoadSegment()->getStart();
	travelMetric.origin = WayPoint(startNode);
	travelMetric.started = true;

	//cbd
	travelMetric.cbdTraverseType = getParent()->currSubTrip->cbdTraverseType;
	switch(travelMetric.cbdTraverseType)
	{
	case TravelMetric::CBD_ENTER:
		break;
	case TravelMetric::CBD_EXIT:
		travelMetric.cbdOrigin = travelMetric.origin;
		travelMetric.cbdStartTime = travelMetric.startTime;
		break;
	};
	return  travelMetric;
}

TravelMetric& sim_mob::medium::DriverMovement::finalizeTravelTimeMetric()
{
	if(!travelMetric.started) { return travelMetric; } //sanity check
	if(pathMover.getPath().empty())
	{
		Print() << "Person " << getParent()->getId() << " has no path\n";
		return  travelMetric;
	}

	const sim_mob::SegmentStats * currSegStat = ((pathMover.getCurrSegStats() == nullptr) ? *(pathMover.getPath().rbegin()) : (pathMover.getCurrSegStats()));
	const Node* endNode = currSegStat->getRoadSegment()->getEnd();
	travelMetric.destination = WayPoint(endNode);
	travelMetric.endTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
	travelMetric.travelTime = TravelMetric::getTimeDiffHours(travelMetric.endTime , travelMetric.startTime);
	travelMetric.finalized = true;

	//cbd
	sim_mob::RestrictedRegion &cbd = sim_mob::RestrictedRegion::getInstance();
	switch(travelMetric.cbdTraverseType)
	{
	case TravelMetric::CBD_ENTER:
		travelMetric.cbdDestination = travelMetric.destination;
		travelMetric.cbdEndTime = travelMetric.endTime;
		travelMetric.cbdTravelTime = TravelMetric::getTimeDiffHours(travelMetric.cbdEndTime , travelMetric.cbdStartTime);
		break;
	case TravelMetric::CBD_EXIT:
		break;
	};

//	if(travelMetric.cbdTraverseType == sim_mob::TravelMetric::CBD_ENTER ||
//			travelMetric.cbdTraverseType == sim_mob::TravelMetric::CBD_EXIT)
//	{
//		getParent()->serializeCBD_SubTrip(*travelMetric);
//	}
//	getParent()->addSubtripTravelMetrics(*travelMetric);
	return  travelMetric;
}


TravelMetric& DriverMovement::processCBD_TravelMetrics(const sim_mob::RoadSegment* completedRS, const sim_mob::RoadSegment* nextRS)
{
	//	the following conditions should hold in order to process CBD data
	TravelMetric::CDB_TraverseType type = travelMetric.cbdTraverseType;
	bool proceed = (nextRS && !pathMover.isPathCompleted());
	if(!proceed)
	{
		return	travelMetric;
	}

	std::string now((DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime()).getStrRepr());
	sim_mob::RestrictedRegion &cbd = sim_mob::RestrictedRegion::getInstance();
	std::stringstream out("");

	//	update travel distance
	if(cbd.isInRestrictedSegmentZone(completedRS))
	{
		travelMetric.cbdDistance += completedRS->getPolylineLength();
	}

	//process either enter or exit
	switch(type)
	{
	case TravelMetric::CBD_ENTER:{
		//search if you are about to enter CBD (we assume the trip started outside cbd and  is going to end inside cbd)
		if(!cbd.isInRestrictedSegmentZone(completedRS)&&cbd.isInRestrictedSegmentZone(nextRS) && travelMetric.cbdEntered.check())
		{
			out << getParent()->getId() << "onSegmentCompleted Enter CBD " << completedRS->getId() << "," << (nextRS ? nextRS->getId() : 0) << "\n";
			travelMetric.cbdOrigin = sim_mob::WayPoint(completedRS->getEnd());
			travelMetric.cbdStartTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
		}
		break;
	}
	case TravelMetric::CBD_EXIT:{
		//search if you are about to exit CBD(we assume the trip started inside cbd and is going to end outside cbd)
		if(cbd.isInRestrictedSegmentZone(completedRS)&&!cbd.isInRestrictedSegmentZone(nextRS) && travelMetric.cbdExitted.check())
		{
			out << getParent()->getId() << "onSegmentCompleted exit CBD " << completedRS->getId() << "," << (nextRS ? nextRS->getId() : 0) << "\n";
			travelMetric.cbdDestination = sim_mob::WayPoint(completedRS->getEnd());
			travelMetric.cbdEndTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
			travelMetric.cbdTravelTime = sim_mob::TravelMetric::getTimeDiffHours(travelMetric.cbdEndTime , travelMetric.cbdStartTime);
		}
		break;
	case TravelMetric::CBD_PASS:{
		if(!cbd.isInRestrictedSegmentZone(completedRS)&&cbd.isInRestrictedSegmentZone(nextRS) && travelMetric.cbdEntered.check())
		{
			out << getParent()->getId() << "onSegmentCompleted Pass Enter CBD " << completedRS->getId() << "," << (nextRS ? nextRS->getId() : 0) << "\n";
			travelMetric.cbdOrigin = sim_mob::WayPoint(completedRS->getEnd());
			travelMetric.cbdStartTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
		}
		if(cbd.isInRestrictedSegmentZone(completedRS)&&!cbd.isInRestrictedSegmentZone(nextRS))
		{
			out << getParent()->getId() << "onSegmentCompleted Pass exit CBD " << completedRS->getId() << "," << (nextRS ? nextRS->getId() : 0) << "\n";
			travelMetric.cbdDestination = sim_mob::WayPoint(completedRS->getEnd());
			travelMetric.cbdEndTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
			travelMetric.cbdTravelTime = sim_mob::TravelMetric::getTimeDiffHours(travelMetric.cbdEndTime , travelMetric.cbdStartTime);
		}
		break;
	}
	}
	};
//	std::cout << out.str() ;
	return travelMetric;
}

int DriverMovement::findReroutingPoints(const std::vector<sim_mob::SegmentStats*>& stats,
		std::map<const sim_mob::Node*, std::vector<const sim_mob::SegmentStats*> >& remaining) const{

	//some variables and iterators before the Actual Operation
	const std::vector<const sim_mob::SegmentStats*> & path = getMesoPathMover().getPath(); //driver's current path
	std::vector<const sim_mob::SegmentStats*>::const_iterator startIt = std::find(path.begin(), path.end(), getMesoPathMover().getCurrSegStats());//iterator to driver's current location
	std::vector<const sim_mob::SegmentStats*>::const_iterator endIt = std::find(path.begin(), path.end(), *(stats.begin()));//iterator to incident segstat
	std::vector<const sim_mob::SegmentStats*> rem;//stats remaining from the current location to the re-routing point
	//Actual Operation : As you move from your current location towards the incident, store the intersections on your way + the segstats you travrsed until you reach that intersection.
//	//debug
//	pathsetLogger << "Original Path:" << std::endl;
//	MesoPathMover::printPath(path);
//	//debug...
	for(const sim_mob::Link * currLink = (*startIt)->getRoadSegment()->getLink() ;startIt <= endIt; startIt++)
	{
		//record the remaining segstats
		rem.push_back(*startIt);
		//link changed?
		if(currLink != (*startIt)->getRoadSegment()->getLink()){
			//record
			remaining[currLink->getEnd()] = rem;//no need to clear rem!
			//last segment lies in the next link, remove it
			remaining[currLink->getEnd()].pop_back();
			//update the current iteration link
			currLink = (*startIt)->getRoadSegment()->getLink();
		}
	}
	//filter out no paths
	std::map<const sim_mob::Node*, std::vector<const sim_mob::SegmentStats*> >::iterator noPathIt = remaining.begin();
	while (noPathIt != remaining.end()) {
	   if (!(noPathIt->second.size()))
	      remaining.erase(noPathIt++);
	   else
		   noPathIt++;
	}
	pathsetLogger << "-------------------------------------------\n" <<
			"Candidates with their remaining path after filtering the no paths:" << std::endl;
	typedef std::map<const sim_mob::Node*, std::vector<const sim_mob::SegmentStats*> >::value_type TempType;
	BOOST_FOREACH(TempType &item,  remaining){
		pathsetLogger << "Remaining path to detour point : ";
		pathsetLogger << MesoPathMover::printPath(item.second, item.first);
	}
	pathsetLogger << "\n-------------------------------------------" << std::endl;
	pathsetLogger << "There are " << remaining.size() << " candidate point of reroute for Person(excluding no path):" << std::endl;
	return remaining.size();
}

/*here is how we detect UTurns. If
	//S1 is the 'last' segment of the old path with O1 and D1 as the start and end node respectively, and
	//S2 is the 'first' segment of the new path with O2 and D2 as the start and end node respectively,
	//if the following condition holds, we have a UTurn:
	// (O1==D2) && (D2 == O1)  make sense?
*/
bool DriverMovement::hasUTurn(std::vector<WayPoint> & newPath, std::vector<const sim_mob::SegmentStats*> & oldPath){

 const sim_mob::Node *O_new = newPath.begin()->roadSegment_->getStart();
 const sim_mob::Node *D_new = newPath.begin()->roadSegment_->getEnd();
 const sim_mob::Node *O_old = (*oldPath.rbegin())->getRoadSegment()->getStart();//using .begin() or .end() makes no difference
 const sim_mob::Node *D_old = (*oldPath.rbegin())->getRoadSegment()->getEnd();

 if((O_old == D_new) && (D_old == O_new)){
	 return true;
 }
 return false;
}

bool DriverMovement::UTurnFree(std::vector<WayPoint> & newPath, std::vector<const sim_mob::SegmentStats*> & oldPath , sim_mob::SubTrip &subTrip, std::set<const sim_mob::RoadSegment*> & excludeRS){
	pathsetLogger<< "UTurn detected" << std::endl;
	if(!hasUTurn(newPath, oldPath)){
		return true;
	}
	//exclude/blacklist the UTurn segment on the new path(first segment)
	excludeRS.insert((*newPath.begin()).roadSegment_);
	//create a path using updated black list
	//and then try again
	//try to remove UTurn by excluding the segment (in the new part of the path) from the graph and regenerating pathset
	//if no path, return false, if path found, return true
	std::stringstream outDbg("");
	sim_mob::PathSetManager::getInstance()->getBestPath(newPath,subTrip, true,  excludeRS,false,false,false,nullptr);
	//try again
	if(!newPath.size()){
		pathsetLogger<< "No other path can avoid a Uturn, suggest to discard \n" ;
		return false;//wasn't successful, so return false
	}

	if(hasUTurn(newPath, oldPath)){
		throw std::runtime_error("UTurn detected where the corresponding segment involved in the UTurn is already excluded");
	}
	pathsetLogger<< "New Path generated to avoid a UTurn" << std::endl;
	return true;
}

bool DriverMovement::canJoinPaths(std::vector<WayPoint> & newPath, std::vector<const sim_mob::SegmentStats*> & oldPath
		, sim_mob::SubTrip &subTrip, std::set<const sim_mob::RoadSegment*> & excludeRS){

	 const sim_mob::RoadSegment *from = (*oldPath.rbegin())->getRoadSegment();//using .begin() or .end() makes no difference
	 const sim_mob::RoadSegment *to = newPath.begin()->roadSegment_;
	 if(isConnectedToNextSeg(from,to))
	 {
		 return true;
	 }
	 //now try to find another path
	 pathsetLogger << "No connection between the old&new paths. reTrying to join paths by excluding segment : " << (*newPath.begin()).roadSegment_->getSegmentAimsunId() << std::endl;
//	MesoPathMover::printPath(oldPath);
//	printWPpath(newPath);

	//exclude/blacklist the segment on the new path(first segment)
	excludeRS.insert((*newPath.begin()).roadSegment_);
	//create a path using updated black list
	//and then try again
	//try to remove UTurn by excluding the segment (in the new part of the path) from the graph and regenerating pathset
	//if no path, return false, if path found, return true
	std::stringstream outDbg("");
	sim_mob::PathSetManager::getInstance()->getBestPath(newPath,subTrip, true, excludeRS,false,false,false,nullptr);
	to = newPath.begin()->roadSegment_;
	bool res = isConnectedToNextSeg(from,to);
	return res;
}

//todo put this in the utils(and code style!)
boost::mt19937 myOwngen;
int roll_die(int l,int r) {
    boost::uniform_int<> dist(l,r);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(myOwngen, dist);
    return die();
}

void DriverMovement::reroute()
{
	rerouter->reroute();
}

//step-1: can I rerout? if yes, what are my points of rerout?
//step-2: do I 'want' to reroute?
//step-3: get a new path from each candidate re-routing points
//step-4: In order to get to the detour point, some part of the original path should still be traveled. prepend that part to the new paths
//setp-5: setpath: assign the assembled path to pathmover
void DriverMovement::reroute(const InsertIncidentMessage &msg){
	pathsetLogger << "rerouting" << std::endl;
	//step-1
	std::map<const sim_mob::Node*, std::vector<const sim_mob::SegmentStats*> > deTourOptions ; //< detour point, segments to travel before getting to the detour point>
	deTourOptions.clear(); // :)
	int numReRoute = findReroutingPoints(msg.stats, deTourOptions);
	if(!numReRoute){
		return;
	}

	//step-2
	if(!wantReRoute()){
		return;
	}
	pathsetLogger << numReRoute << "Rerouting Points were identified" << std::endl;
	//step-3:
	typedef std::map<const sim_mob::Node*, std::vector<const sim_mob::SegmentStats*> >::value_type	DetourOption ; //for 'deTourOptions' container
	std::set<const sim_mob::RoadSegment*> excludeRS = std::set<const sim_mob::RoadSegment*>();
	//	get a 'copy' of the person's current subtrip
	SubTrip subTrip = *(getParent()->currSubTrip);
	std::map<const sim_mob::Node* , std::vector<WayPoint> > newPaths ; //stores new paths starting from the re-routing points
	BOOST_FOREACH(DetourOption detourNode, deTourOptions)
	{
		// change the origin
		//todo and the start time !!!-vahid
		subTrip.fromLocation.node_ = detourNode.first;
		//	record the new paths using the updated subtrip. (including no paths)
		sim_mob::PathSetManager::getInstance()->getBestPath(newPaths[detourNode.first], subTrip,true, std::set<const sim_mob::RoadSegment*>(), false,false,false,nullptr);//partially excluded sections must be already added
	}

	/*step-4: prepend the old path to the new path
	 * old path: part of the originalpathsetLogger path from the agent's current position to the rerouting point
	 * new path:the path from the rerouting point to the destination
	 * Note: it is more efficient to do this within the above loop but code reading will become more tough*/
	//4.a: check if there is no path from the rerouting point, just discard it.
	//4.b: check and discard the rerouting point if the new and old paths can be joined
	//4.c convert waypoint to segstat and prepend(join) remaining oldpath to the new path
	typedef std::map<const sim_mob::Node* , std::vector<WayPoint> >::value_type NewPath;
	BOOST_FOREACH(NewPath &newPath, newPaths)
	{
		//4.a
		if(newPath.second.empty()){
			Warn() << "No path on Detour Candidate node " << newPath.first->getID() << std::endl;
			deTourOptions.erase(newPath.first);
			continue;
		}
		//4.b
		// change the origin
		subTrip.fromLocation.node_ = newPath.first;
		pathsetLogger<< "Try Joining old and new paths for detour point :" << newPath.first->getID() << std::endl;
//		MesoPathMover::printPath(deTourOptions[newPath.first], newPath.first);
//		printWPpath(newPath.second, newPath.first);
		//check if join possible
		bool canJoin = canJoinPaths(newPath.second,deTourOptions[newPath.first], subTrip, excludeRS);
		if(!canJoin)
		{
			pathsetLogger << "could not join the old and new paths, discarding detour point :" << newPath.first->getID() << std::endl;
//			sim_mob::printWPpath(newPath.second, newPath.first);
			deTourOptions.erase(newPath.first);
			continue;
		}
		pathsetLogger << "Paths can Join" << std::endl;
		//4.c join
		initSegStatsPath(newPath.second,deTourOptions[newPath.first]);

		//step-4.d cancel similar paths
		//some newPath(s) can be subset of the other path(s).
		//This can be easily detected when the old part of path and the new path join: it can create a combination that has already been created
		//so let's look for 'same paths':
		std::vector<const sim_mob::SegmentStats*> & target = deTourOptions[newPath.first];
		BOOST_FOREACH(DetourOption &detourNode, deTourOptions)
		{
			//dont compare with yourself
			if(detourNode.first == newPath.first){continue;}
			if(target == detourNode.second)
			{
				pathsetLogger << "Discarding an already been created path:\n";
				pathsetLogger << MesoPathMover::printPath(detourNode.second);
				pathsetLogger << MesoPathMover::printPath(target);
				deTourOptions.erase(newPath.first);
			}
//			//if they have a different size, they are definitely different,so leave this entry alone
//			if(target.size() != detourNode.second.size()){continue;}
//			typedef std::vector<const sim_mob::SegmentStats*>::const_iterator it_;
//			std::pair<it_,it_> comp = std::mismatch(target.begin(),target.end(), detourNode.second.begin(), detourNode.second.end());
			//since the two containers have the same size, they are considered equal(same) if any element of the above pair is equal to the .end() of their corresponding containers
//			if (comp.first == target.end())
//			{
//				pathsetLogger << "Discarding an already been created path:" << std::endl;
//				MesoPathMover::printPath(detourNode.second);
//				MesoPathMover::printPath(target);
//				deTourOptions.erase(newPath.first);
//			}
		}
	}
	//is there any place drivers can re-route or not?
	if(!deTourOptions.size()){
		pathsetLogger << "No Detour For incident at " << (*msg.stats.begin())->getRoadSegment()->getSegmentAimsunId() << std::endl;
		return;
	}

	//step-5: now you may set the path using 'deTourOptions' container
	//todo, put a distribution function here. For testing now, give it the last new path for now
	std::map<const sim_mob::Node*, std::vector<const sim_mob::SegmentStats*> >::iterator it(deTourOptions.begin());

	int cnt = roll_die(0,deTourOptions.size() - 1);
	int dbgIndx = cnt;
	while(cnt){ it++; --cnt;}
	//debug
	pathsetLogger << "----------------------------------\n"
			"Original path:" << std::endl;
	pathsetLogger << getMesoPathMover().printPath(getMesoPathMover().getPath());
	pathsetLogger << "Detour option chosen[" << dbgIndx << "] : " << it->first->getID() << std::endl;
	pathsetLogger << getMesoPathMover().printPath(it->second);
	pathsetLogger << "----------------------------------" << std::endl;
	//debug...
	getMesoPathMover().setPath(it->second);
}

sim_mob::Conflux* DriverMovement::getStartingConflux() const
{
	const sim_mob::SegmentStats* firstSegStats = pathMover.getCurrSegStats(); //first segstats of the remaining path.
	return firstSegStats->getRoadSegment()->getParentConflux();
}

void DriverMovement::handleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type){
	case MSG_INSERT_INCIDENT:{
		const InsertIncidentMessage &msg = MSG_CAST(InsertIncidentMessage,message);
		PathSetManager::getInstance()->addPartialExclusion((*msg.stats.begin())->getRoadSegment());
		reroute(msg);
		break;
	}
	}
}

const sim_mob::Link* DriverMovement::getNextLinkForLaneChoice(const SegmentStats* nextSegStats) const
{
	const sim_mob::Link* nextLink = nullptr;
	const sim_mob::SegmentStats* firstStatsInNextLink = pathMover.getFirstSegStatsInNextLink(nextSegStats);
	if(firstStatsInNextLink) { nextLink = firstStatsInNextLink->getRoadSegment()->getLink(); }
	return nextLink;
}

} /* namespace medium */
} /* namespace sim_mob */


