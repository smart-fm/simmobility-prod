#include <math.h>
#include "entities/roles/pedestrian/Pedestrian.hpp"
//#include "entities/roles/driver/BusDriver.hpp"
#include "Driver.hpp"
#include "entities/Person.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/TripChain.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Point2D.hpp"
#include "util/OutputUtil.hpp"
#include "util/DynamicVector.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DebugFlags.hpp"
#include "partitions/PartitionManager.hpp"
#include "entities/AuraManager.hpp"

#include <ostream>

#ifndef SIMMOB_DISABLE_PI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#endif

using namespace sim_mob;
using namespace sim_mob::medium;
using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;

//Helper functions
namespace {
//Helpful constants

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

//TO DO: not implemented yet
double getOutputCounter(const Lane* l) {
	return -1.0;
}

//TO DO: not implemented yet
double getOutputFlowRate(const Lane* l) {
	return -1.0;
}

//TO DO: not implemented yet
double getQueueLength(const Lane* l) {
	return -1.0;
}

} //end of anonymous namespace

//Initialize
sim_mob::medium::Driver::Driver(Agent* parent, MutexStrategy mtxStrat) :
	Role(parent), currLane_(mtxStrat, nullptr), currLaneOffset_(mtxStrat, 0), currLaneLength_(mtxStrat, 0), isInIntersection(mtxStrat, false),
	fwdVelocity(mtxStrat,0), vehicle(nullptr), params(parent->getGenerator())
{

//	if (Debug::Drivers) {
//		DebugStream << "Driver starting: " << parent->getId() << endl;
//	}
	vehicle = nullptr;
	nextLaneInNextLink = nullptr;
	intModel = new SimpleIntDrivingModel();

	remainingTimeToComplete = 0;
	targetLaneIndex = 0;

}

///Note that Driver's destructor is only for reclaiming memory.
///  If you want to remove its registered properties from the Worker (which you should do!) then
///  this should occur elsewhere.
sim_mob::medium::Driver::~Driver() {
	//Our vehicle
	safe_delete_item(vehicle);

	safe_delete_item(intModel);
	ss << "!!__________________________________________!!" << endl;
	std::cout << ss.str();
}

vector<BufferedBase*> sim_mob::medium::Driver::getSubscriptionParams() {
	vector<BufferedBase*> res;
	res.push_back(&(currLane_));
	res.push_back(&(currLaneOffset_));
	res.push_back(&(currLaneLength_));
	res.push_back(&(isInIntersection));
	res.push_back(&(fwdVelocity));
	return res;
}

Role* sim_mob::medium::Driver::clone(Person* parent) const
{
	return new Driver(parent, parent->getMutexStrategy());
}

void sim_mob::medium::Driver::frame_init(UpdateParams& p)
{
	//Save the path from orign to next activity location in allRoadSegments
	sim_mob::medium::MidVehicle* newVeh = initializePath(true);
	if (newVeh) {
		safe_delete_item(vehicle);
		vehicle = newVeh;
	}

	//Set some properties about the current path, such as the current polyline, etc.
	if (vehicle && vehicle->hasPath()) {
		setOrigin(params);
	} else {
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout << "ERROR: Vehicle could not be created for driver; no route!\n";
#endif
	}

	//Updating location information for agent for density calculations
	parent->setCurrLane(params.currLane);
	parent->setCurrLink((params.currLane)->getRoadSegment()->getLink());

	ss << "!!!!!!!!!!!!!!!!!!!!!!!!!!  " << this->parent->getId() << "  !!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
}

void sim_mob::medium::Driver::setOrigin(DriverUpdateParams& p) {

	//Set our current and target lanes.
	p.currLane = vehicle->getCurrLane();
	p.currLaneIndex = getLaneIndex(p.currLane);
	targetLaneIndex = p.currLaneIndex;

	//Vehicles start at rest
	vehicle->setVelocity(0);

	//Calculate and save the total length of the current polyline.
	p.currLaneLength = vehicle->getCurrLinkLaneZeroLength();

	//if the first road segment is the last one in this link
	if (!vehicle->hasNextSegment(true)) {
		//saveCurrTrafficSignal();
	}
	if (!vehicle->hasNextSegment(true) && vehicle->hasNextSegment(false)) {
		//Don't do this if there is no next link.
		chooseNextLaneForNextLink(p);
	}
}

sim_mob::UpdateParams& sim_mob::medium::Driver::make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS)
{
	params.reset(frameNumber, currTimeMS, *this);
	return params;
}

void sim_mob::medium::DriverUpdateParams::reset(frame_t frameNumber, unsigned int currTimeMS, const Driver& owner)
{
	UpdateParams::reset(frameNumber, currTimeMS);

	//Set to the previous known buffered values
	currLane = owner.currLane_.get();
	currLaneIndex = getLaneIndex(currLane);
	currLaneLength = owner.currLaneLength_.get();
	currLaneOffset = owner.currLaneOffset_.get();
	nextLaneIndex = currLaneIndex;

	//Reset; these will be set before they are used; the values here represent either defaul
	//       values or are unimportant.
	currSpeed = 0;

	elapsedSeconds = ConfigParams::GetInstance().baseGranMS / 1000.0;

	timeSpent = 0.0;

	//Set to true if we have just moved to a new segment.
	justChangedToNewSegment = false;

	//Will be removed later.
	TEMP_lastKnownPolypoint = DPoint(0, 0);

	//Set to true if we have just moved into an intersection.
	justMovedIntoIntersection = false;

	//If we've just moved into an intersection, is set to the amount of overflow (e.g.,
	//  how far into it we already are.)
	overflowIntoIntersection = 0;
}

//Main update functionality
//~melani
void sim_mob::medium::Driver::frame_tick(UpdateParams& p)
{
	DriverUpdateParams& p2 = dynamic_cast<DriverUpdateParams&>(p);

	/* To be added by supply team to update location of driver after every frame tick
	 * remember
	 * 		1. Update all the activity parameters of the agent after every ACTIVITY_END event,
	 * 		2. Update the nextPathPlanned flag to indicate whether agent needs to request for next detailed path
	 */
	//DriverUpdateParams& p2 = dynamic_cast<DriverUpdateParams&>(p);

	//Are we done already?
	if (vehicle->isDone()) {
		parent->setToBeRemoved();
		return;
	}

	//General update behavior.
	//Note: For now, most updates cannot take place unless there is a Lane and vehicle.
	if (p2.currLane && vehicle) {
		if (update_movement(p2, p.frameNumber) && update_post_movement(p2, p.frameNumber)) {

			//Update parent data. Only works if we're not "done" for a bad reason.
			setParentBufferedData();
		}
	}


	//Update our Buffered types
	if (!vehicle->isInIntersection()) {
		//currLane_ = vehicle->getCurrLane();
		//currLaneOffset_ = vehicle->getDistanceMovedInSegment();
		//currLaneLength_ = vehicle->getCurrLinkLaneZeroLength();
		currLane_.set(vehicle->getCurrLane());
		currLaneOffset_.set(vehicle->getDistanceMovedInSegment());
		currLaneLength_.set(vehicle->getCurrLinkLaneZeroLength());
	}

	isInIntersection.set(vehicle->isInIntersection());
	fwdVelocity.set(vehicle->getVelocity());
}

void sim_mob::medium::Driver::setParentBufferedData() {
	parent->xPos.set(vehicle->getX());
	parent->yPos.set(vehicle->getY());

	//TODO: Need to see how the parent agent uses its velocity vector.
	parent->fwdVel.set(vehicle->getVelocity());
}

//TODO: For now, we're just using a simple trajectory model. Complex curves may be added later.
void sim_mob::medium::Driver::calculateIntersectionTrajectory(DPoint movingFrom, double overflow) {
	//If we have no target link, we have no target trajectory.
	if (!nextLaneInNextLink) {
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout << "WARNING: nextLaneInNextLink has not been set; can't calculate intersection trajectory."
				<< std::endl;
#endif
		return;
	}

	//Get the entry point.
	Point2D entry = nextLaneInNextLink->getPolyline().at(0);

	//Compute a movement trajectory.
	intModel->startDriving(movingFrom, DPoint(entry.getX(), entry.getY()), overflow);
}

//~melani
bool sim_mob::medium::Driver::update_movement(DriverUpdateParams& params, frame_t frameNumber) {
	//If reach the goal, get back to the origin
	if (vehicle->isDone()) {
		//Output
		if (Debug::Drivers && !DebugStream.str().empty()) {
#ifndef SIMMOB_DISABLE_OUTPUT
			DebugStream << ">>>Vehicle done." << endl;
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			std::cout << DebugStream.str();
			DebugStream.str("");
#endif
		}

		return false;
	}

	//Save some values which might not be available later.
	const RoadSegment* prevSegment = vehicle->getCurrSegment();

	params.TEMP_lastKnownPolypoint = DPoint(vehicle->getCurrPolylineVector().getEndX(),
				vehicle->getCurrPolylineVector().getEndY());

	//if (vehicle->isQueuing()) {
		//Queuing not implemented yet. ~melani
	//}

	if (vehicle->isInIntersection()) {
			intersectionDriving(params);
		}

	//Next, handle driving on links.
	// Note that a vehicle may leave an intersection during intersectionDriving(), so the conditional check is necessary.
	// Note that there is no need to chain this back to intersectionDriving.
	if (!vehicle->isInIntersection()) {
		params.overflowIntoIntersection = updatePositionOnLink(params);
		//Did our last move forward bring us into an intersection?
		if (vehicle->isInIntersection()) {
			params.justMovedIntoIntersection = true;
		}
	}

	//Has the segment changed?
	if (!vehicle->isDone()) {
		params.justChangedToNewSegment = (vehicle->getCurrSegment() != prevSegment);
	}
	return true;
}

//~melani
void sim_mob::medium::Driver::intersectionDriving(DriverUpdateParams& p) {
	//Don't move if we have no target
	if (!nextLaneInNextLink) {
		return;
	}

	//First, update movement along the vector.
	DPoint res = intModel->continueDriving(vehicle->getVelocity() * p.elapsedSeconds);
	vehicle->setPositionInIntersection(res.x, res.y);

	//Next, detect if we've just left the intersection. Otherwise, perform regular intersection driving.
	if (intModel->isDone()) {
		p.currLane = vehicle->moveToNextSegmentAfterIntersection();
		justLeftIntersection(p);
	}
}

void sim_mob::medium::Driver::justLeftIntersection(DriverUpdateParams& p) {
	p.currLane = nextLaneInNextLink;
	p.currLaneIndex = getLaneIndex(p.currLane);
	vehicle->moveToNewLanePolyline(p.currLaneIndex);
	syncCurrLaneCachedInfo(p);
	p.currLaneOffset = vehicle->getDistanceMovedInSegment();
	targetLaneIndex = p.currLaneIndex;

	//Updating location information for agent for density calculations
	parent->setCurrLane(p.currLane);
	parent->setCurrLink((p.currLane)->getRoadSegment()->getLink());

}

//General update information for whenever a Segment may have changed.
void sim_mob::medium::Driver::syncCurrLaneCachedInfo(DriverUpdateParams& p) {
	//The lane may have changed; reset the current lane index.
	p.currLaneIndex = getLaneIndex(p.currLane);

	//Update which lanes are adjacent.
	//updateAdjacentLanes(p);

	//Update the length of the current road segment.
	p.currLaneLength = vehicle->getCurrLinkLaneZeroLength();

	//Finally, update target/max speed to match the new Lane's rules.
	//maxLaneSpeed = vehicle->getCurrSegment()->maxSpeed / 3.6; //slow down
	//targetSpeed = maxLaneSpeed;
}

void sim_mob::medium::Driver::frame_tick_output(const UpdateParams& p)
{
	//Skip?
	if (vehicle->isDone() || ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

	double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();

#ifndef SIMMOB_DISABLE_OUTPUT
	LogOut("(\"Driver\""
			<<","<<p.frameNumber
			<<","<<parent->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(vehicle->getX())
			<<"\",\"yPos\":\""<<static_cast<int>(vehicle->getY())
			<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
			<<"\",\"length\":\""<<static_cast<int>(vehicle->length)
			<<"\",\"width\":\""<<static_cast<int>(vehicle->width)
			<<"\"})"<<std::endl);
#endif

	//Updating location information for agent for density calculations
	parent->setCurrLane(params.currLane);
	parent->setCurrLink((params.currLane)->getRoadSegment()->getLink());
}

double sim_mob::medium::Driver::updatePositionOnLink(DriverUpdateParams& p) {
	//Determine how far forward we've moved.

	// Fetch number of moving vehicles in the segment and compute speed from the speed density function
	std::map<const sim_mob::Lane*, unsigned short> movingCounts =
				AuraManager::instance().getMovingCountsOfLanes(vehicle->getCurrSegment());
	vehicle->setVelocity(speed_density_function(movingCounts));

	double fwdDistance = vehicle->getVelocity() * p.elapsedSeconds;
	if (fwdDistance < 0)
		fwdDistance = 0;

	//Move the vehicle forward.
	double res = 0.0;
	try {
		res = vehicle->moveFwd(fwdDistance);
	} catch (std::exception& ex) {
		if (Debug::Drivers) {
#ifndef SIMMOB_DISABLE_OUTPUT
			DebugStream << ">>>Exception: " << ex.what() << endl;
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			std::cout << DebugStream.str();
#endif
		}

		std::stringstream msg;
		msg << "Error moving vehicle forward for Agent ID: " << parent->getId() << "," << this->vehicle->getX() << "," << this->vehicle->getY() << "\n" << ex.what();
		throw std::runtime_error(msg.str().c_str());
	}

	//Update our offset in the current lane.
	if (!vehicle->isInIntersection()) {
		p.currLaneOffset = vehicle->getDistanceMovedInSegment();
	}
	return res;
}

//currently it just chooses the first lane from the targetLane
//Note that this also sets the target lane so that we (hopefully) merge before the intersection.
void sim_mob::medium::Driver::chooseNextLaneForNextLink(DriverUpdateParams& p) {
	//p.nextLaneIndex = p.currLaneIndex;
	//Retrieve the node we're on, and determine if this is in the forward direction.
	const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (vehicle->getNodeMovingTowards());
	const RoadSegment* nextSegment = vehicle->getNextSegment(false);

/*	//Build up a list of target lanes.
	nextLaneInNextLink = nullptr;
	vector<const Lane*> targetLanes;
	if (currEndNode && nextSegment) {
		const set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(*vehicle->getCurrSegment());
		std::cout << "Segment : " << vehicle->getCurrSegment()<< std::endl;
		for (set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++) {
			std::cout<< (*it)->getLaneTo()->getLaneID_str() << ":" << (*it)->getLaneTo()->getLaneID() << std::endl;
			if ((*it)->getLaneTo()->getRoadSegment() == nextSegment &&
					(*it)->getLaneFrom() == currLane_) {
				if (!((*it)->getLaneTo()->is_pedestrian_lane())) {
					targetLanes.push_back((*it)->getLaneTo());
				}
			}
		}
	}
*/
	//getBestTargetLane();

	//====
	//Build up a list of target lanes.
	nextLaneInNextLink = nullptr;
	vector<const Lane*> targetLanes;
	if (currEndNode && nextSegment) {
		const set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(*vehicle->getCurrSegment());
		for (set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++) {
			if ((*it)->getLaneTo()->getRoadSegment() == nextSegment && (*it)->getLaneFrom() == currLane_) {
				//It's a valid lane.
				targetLanes.push_back((*it)->getLaneTo());

				//find target lane with same index, use this lane
				size_t laneIndex = getLaneIndex((*it)->getLaneTo());
				if (laneIndex == p.currLaneIndex&&!((*it)->getLaneTo()->is_pedestrian_lane())){
					nextLaneInNextLink = (*it)->getLaneTo();
					targetLaneIndex = laneIndex;
					break;
				}
			}
		}

		//Still haven't found a lane?
		if (!nextLaneInNextLink) {
			//no lane with same index, use the first lane in the vector if possible.
			if (targetLanes.size() > 0) {
				nextLaneInNextLink = targetLanes.at(0);
				if(nextLaneInNextLink->is_pedestrian_lane()&&targetLanes.size()>1)
					nextLaneInNextLink = targetLanes.at(1);
				targetLaneIndex = getLaneIndex(nextLaneInNextLink);
			} else if (nextSegment) { //Fallback
				size_t fallbackIndex = std::min(p.currLaneIndex, nextSegment->getLanes().size() - 1);
				nextLaneInNextLink = nextSegment->getLanes().at(fallbackIndex);
				targetLaneIndex = fallbackIndex;
			}
		}

		//We should have generated a nextLaneInNextLink here.
		if (!nextLaneInNextLink) {
			throw std::runtime_error("Can't find nextLaneInNextLink.");
		}
	}
}

//~melani - TO DO
void sim_mob::medium::Driver::getBestTargetLane(const vector<const Lane*> targetLanes){

	//1. Get queueing counts for all lanes of the next Segment
	//2. Select the lane with the least queue length
	//3. Update nextLaneInNextLink and targetLaneIndex accordingly

	//int queueingCount = 0;
	//try{
	//		vector< pair<RoadSegment*, bool> = AuraManager::instance().getVehicleCounts(vehicle->getNextSegment());
	//	}
	//	catch (std::exception &e){
	//		queueingCount = 0;
	//	}

	//if (targetLanes.size() > 0) {
	//		for( int i = 0; i < targetLanes.size(); i++ ) {

			   //targetLanes[i]->getQueueLength();
	//		}
	//	}


	//	nextLaneInNextLink = /*(*it)->getLaneTo();*/targetLanes.at(0);
	//	targetLaneIndex = /*getLaneIndex((*it)->getLaneTo())*/0;

		//We should have generated a nextLaneInNextLink here.
	//	if (!nextLaneInNextLink) {
	//		throw std::runtime_error("Can't find nextLaneInNextLink.");
	//	}

}


bool sim_mob::medium::Driver::update_post_movement(DriverUpdateParams& params, frame_t frameNumber) {
	//Are we done?
	if (vehicle->isDone()) {
		return false;
	}

	if (!vehicle->isInIntersection() && !vehicle->hasNextSegment(true) && vehicle->hasNextSegment(false))
		chooseNextLaneForNextLink(params);
	//Have we just entered into an intersection?
	if (vehicle->isInIntersection() && params.justMovedIntoIntersection) {
		//Calculate a trajectory and init movement on that intersection.
		calculateIntersectionTrajectory(params.TEMP_lastKnownPolypoint, params.overflowIntoIntersection);
		intersectionVelocityUpdate();

		//Fix: We need to perform this calculation at least once or we won't have a heading within the intersection.
		DPoint res = intModel->continueDriving(0);
		vehicle->setPositionInIntersection(res.x, res.y);
	}

	return true;
}

void sim_mob::medium::Driver::intersectionVelocityUpdate() {
	double inter_speed = 1000;//10m/s

	//Set velocity for intersection movement.
	vehicle->setVelocity(inter_speed);
}

sim_mob::medium::MidVehicle* sim_mob::medium::Driver::initializePath(bool allocateVehicle) {
	sim_mob::medium::MidVehicle* res = nullptr;

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
			path = StreetDirectory::instance().shortestDrivingPath(*origin.node, *goal.node);
		}
		//TODO: Start in lane 0?
		int startlaneID = 0;

		// Bus should be at least 1200 to be displayed on Visualizer
		const double length = 400;
		const double width = 200;



		//A non-null vehicle means we are moving.
		if (allocateVehicle) {
			res = new sim_mob::medium::MidVehicle(path, startlaneID, length, width);
		}
	}

	//to indicate that the path to next activity is already planned
	parent->setNextPathPlanned(true);
	return res;
}

//This function is associated with the driver class for 2 reasons
// 1. This function is specific to the medium term
// 2. It makes sense in the real life as well that the driver decides to slow down or accelerate based on the traffic density around him
double sim_mob::medium::Driver::speed_density_function(std::map<const sim_mob::Lane*, unsigned short> laneWiseMovingVehicleCounts) {
	/*
	 * TODO: The parameters - min density, jam density, alpha and beta - for each road segment
	 * must be obtained from an external source (XML/Database)
	 * Since we don't have this data, we have taken the average values from supply parameters of Singapore expressways.
	 * This must be changed after we have this data for each road segment in the network.
	 *
	 * TODO: A params struct for these parameters is already defined in the RoadSegment class.
	 * This struct is to be used when we have actual values for the parameters.
	 */

/*	unsigned int numVehicles = 0;
	const std::vector<sim_mob::Lane*> requiredLanes = laneGroup->getLanes();
	if(requiredLanes.size() > 0){

		for(std::vector<sim_mob::Lane*>::const_iterator laneIt = requiredLanes.begin();
				laneIt!=requiredLanes.end();
				laneIt++ )
		{
			numVehicles += vehicleCounter->getMovingVehicleCount(*laneIt);
		}
	}
	double density = numVehicles / (vehicleCounter->getRoadSegment()->length / 100.0);

	double freeFlowSpeed = vehicle->getCurrSegment()->maxSpeed / 3.6 * 100; // Converting from Kmph to cm/s
	double jamDensity = 1; //density during traffic jam
	double alpha = 3.75; //Model parameter of speed density function
	double beta = 0.5645; //Model parameter of speed density function
	double minDensity = 0.0048; // minimum traffic density

	//Speed-Density function
	if(density <= minDensity){
		return freeFlowSpeed;
	}
	else {
		//TODO: Remove debugging print statement later. Harish
		ss << "!! " << "density:" << density << "!! " << freeFlowSpeed * pow((1 - pow((density - minDensity)/jamDensity, beta)),alpha) << " !!" << std::endl;
		return freeFlowSpeed * pow((1 - pow((density - minDensity)/jamDensity, beta)),alpha);
	}*/
	return 0.0;
}

void sim_mob::medium::Driver::advance(DriverUpdateParams p){
	//for time calculations in this functions, it would be more accurate to have time moved in current segment
	if (parent->isQueuing)
	{
		//not implemented
		double outCount = getOutputCounter(p.currLane);
		//not implemented
		double outRate = getOutputFlowRate(p.currLane);
		//getCurrSegment length to be tested
		//getDistanceMovedinSeg to be updated with Max's method
		double distToEndOfSegment = vehicle->getCurrSegment()->length - vehicle->getDistanceMovedInSegment();
		double timeLeftToLeaveQ = distToEndOfSegment/(vehicle->length*3.0*outRate);
		if (outCount > 0 && timeLeftToLeaveQ < p.elapsedSeconds)
		{
			//not implemented
			moveToNextSegment(p.elapsedSeconds - timeLeftToLeaveQ);
		}
		else
		{
			//not implemented
			moveInQueue();
		}
	}
	else
	{
		//getNextLinkAndPath();
		//not implemented
		double outCount = getOutputCounter(p.currLane);
		//not implemented
		double outRate = getOutputFlowRate(p.currLane);

		//get current location
		//before checking if the vehicle should be added to a queue, it's re-assigned to the best lane
		//currLane_ = getBestTargetLane();

		//not implemented yet
		double laneQueueLength = getQueueLength(p.currLane);

		if (laneQueueLength > vehicle->getCurrSegment()->length )
		{
			addToQueue();
		}
		else if (laneQueueLength > 0)
		{
			double distToEndOfSegment = vehicle->getCurrSegment()->length - vehicle->getDistanceMovedInSegment();
			double distanceToReachQueue = distToEndOfSegment - laneQueueLength;
			double timeToReachQueue = distanceToReachQueue/vehicle->getVelocity();

			if (timeToReachQueue < p.elapsedSeconds)
			{
				addToQueue();
			}
			else
			{
				//moveForward()
			}
		}
		else if (/*initial*/laneQueueLength > 0)
		{
			double timeToDissipateQ = /*initial*/laneQueueLength/(3.0*outRate*vehicle->length);
			double distToEndOfSegment = vehicle->getCurrSegment()->length - vehicle->getDistanceMovedInSegment();
			double timeToReachEndSeg = distToEndOfSegment/vehicle->getVelocity();
			double tf = std::max(timeToDissipateQ, timeToReachEndSeg);

			if (tf < p.elapsedSeconds)
			{
				if (outCount > 0)
				{
					moveToNextSegment(tf);
				}
				else
				{
					addToQueue();
				}
			}
			else
			{
				//cannot use == operator since it is double variable. tzl, Oct 18, 02
				if( fabs(tf-timeToReachEndSeg) < 0.001 && timeToReachEndSeg > p.elapsedSeconds)
				{
					//tf = mAdvanceInterval;
					//xf = x0-vu*(tf-t0);
					//moveInSegment();
				}
				else
				{
					//xf = 0.0 ;
				}
			}
		}
		else //no queue or no initial queue
		{
			double distToEndOfSegment = vehicle->getCurrSegment()->length - vehicle->getDistanceMovedInSegment();
			double timeToReachEndSeg = distToEndOfSegment/vehicle->getVelocity();
			if (timeToReachEndSeg < p.elapsedSeconds)
			{
				moveToNextSegment(p.elapsedSeconds - timeToReachEndSeg);
			}
			else
			{
				addToQueue();
			}
		}
	}
}


void sim_mob::medium::Driver::moveToNextSegment(double timeLeft)
{

}

void sim_mob::medium::Driver::moveInQueue()
{

}


double sim_mob::medium::Driver::getTimeSpentInTick(DriverUpdateParams p)
{
	return p.timeSpent;
}

void sim_mob::medium::Driver::addToQueue() {
	sim_mob::SegmentVehicles* segVehicles = parent->currWorker->getSegmentVehicles(currResource->getCurrSegment());

	// if agent was moving in this segment then remove from moving list before queuing
	segVehicles->removeMovingAgent(params.currLane, parent);

	// enqueue into the current lane's queue
	segVehicles->addQueuingAgent(params.currLane, parent);
}

void sim_mob::medium::Driver::addToMovingList() {
	sim_mob::SegmentVehicles* segVehicles = parent->currWorker->getSegmentVehicles(currResource->getCurrSegment());
	segVehicles->addMovingAgent(params.currLane, parent);
}

void sim_mob::medium::Driver::removeFromQueue() {
	sim_mob::SegmentVehicles* segVehicles = parent->currWorker->getSegmentVehicles(currResource->getCurrSegment());
	segVehicles->removeQueuingAgent(params.currLane, parent);
}

void sim_mob::medium::Driver::removeFromMovingList() {
	sim_mob::SegmentVehicles* segVehicles = parent->currWorker->getSegmentVehicles(currResource->getCurrSegment());
	segVehicles->removeMovingAgent(params.currLane, parent);
}
