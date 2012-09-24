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


#ifndef SIMMOB_DISABLE_PI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#endif

using namespace sim_mob;

using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;

//Helper functions
namespace {
//Helpful constants

//PathA: Small loop (south)
const Point2D SpecialPathA[] = { Point2D(37218351, 14335255), //AIMSUN 75780
		Point2D(37227139, 14327875), //AIMSUN 91218
		Point2D(37250760, 14355120), //AIMSUN 66508
		Point2D(37241080, 14362955), //AIMSUN 61688
		};

//PathB: Large loop
const Point2D SpecialPathB[] = { Point2D(37218351, 14335255), //AIMSUN 75780
		Point2D(37227139, 14327875), //AIMSUN 91218
		Point2D(37250760, 14355120), //AIMSUN 66508
		Point2D(37270984, 14378959), //AIMSUN 45666
		Point2D(37262150, 14386897), //AIMSUN 45690
		Point2D(37241080, 14362955), //AIMSUN 61688
		};

//Path is in multi-node positions
vector<WayPoint> ConvertToWaypoints(const Node* origin, const vector<Point2D>& path) {
	vector<WayPoint> res;

	//Double-check our first node. Also ensure we have at least 2 nodes (or a path can't be found).
	if (path.size() < 2 || origin->location != path.front()) {
		throw std::runtime_error("Special path does not begin on origin.");
	}

	//Starting at the origin, find the outgoing Link to each node in the list. Then loop around back to the origin.
	const MultiNode* curr = dynamic_cast<const MultiNode*> (origin);
	for (vector<Point2D>::const_iterator it = path.begin(); it != path.end(); it++) {
		if (!curr) {
			throw std::runtime_error("Not a multinode (in special path).");
		}

		//Search for the Link to the next point.
		Point2D nextPt = it + 1 == path.end() ? path.front() : *(it + 1);
		std::pair<const Link*, bool> nextLink(nullptr, false); //bool == fwd?
		const set<RoadSegment*>& segs = curr->getRoadSegments();
		for (set<RoadSegment*>::const_iterator segIt = segs.begin(); segIt != segs.end(); segIt++) {
			const Link* ln = (*segIt)->getLink();
			if (ln->getStart()->location == nextPt) {
				nextLink.first = ln;
				nextLink.second = false;
				break;
			} else if (ln->getEnd()->location == nextPt) {
				nextLink.first = ln;
				nextLink.second = true;
				break;
			}
		}
		if (!nextLink.first) {
			throw std::runtime_error("Couldn't find a Link between nodes in the Special path");
		}

		//Add each Segment in the Link's fwd/rev path to the result.
		const vector<RoadSegment*>& segPath = nextLink.first->getPath(nextLink.second);
		for (vector<RoadSegment*>::const_iterator pthIt = segPath.begin(); pthIt != segPath.end(); pthIt++) {
			res.push_back(WayPoint(*pthIt));
		}

		//Continue
		curr = dynamic_cast<const MultiNode*> (nextLink.second ? nextLink.first->getEnd() : nextLink.first->getStart());
	}

	return res;
}

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

//For the NS3 paths
vector<WayPoint> LoadSpecialPath(const Node* origin, char pathLetter) {
	if (pathLetter == 'A') {
		size_t sz = sizeof(SpecialPathA) / sizeof(SpecialPathA[0]);
		return ConvertToWaypoints(origin, vector<Point2D> (SpecialPathA, &SpecialPathA[sz]));
	} else if (pathLetter == 'B') {
		size_t sz = sizeof(SpecialPathB) / sizeof(SpecialPathB[0]);
		return ConvertToWaypoints(origin, vector<Point2D> (SpecialPathB, &SpecialPathB[sz]));
	} else {
		throw std::runtime_error("Invalid special path.");
	}
}

} //end of anonymous namespace

//Initialize
sim_mob::medium::Driver::Driver(Person* parent, MutexStrategy mtxStrat) :
	Role(parent), currLane_(mtxStrat, nullptr), currLaneOffset_(mtxStrat, 0), currLaneLength_(mtxStrat, 0), isInIntersection(mtxStrat, false),
	fwdVelocity(mtxStrat,0), vehicle(nullptr), params(parent->getGenerator())
{

//	if (Debug::Drivers) {
//		DebugStream << "Driver starting: " << parent->getId() << endl;
//	}
	vehicle = nullptr;
	nextLaneInNextLink = nullptr;
	intModel = new SimpleIntDrivingModel();

}

///Note that Driver's destructor is only for reclaiming memory.
///  If you want to remove its registered properties from the Worker (which you should do!) then
///  this should occur elsewhere.
sim_mob::medium::Driver::~Driver() {
	//Our vehicle
	safe_delete_item(vehicle);

	safe_delete_item(intModel);
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
	Vehicle* newVeh = initializePath(true);
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
	parent->latVel.set(vehicle->getLatVelocity());
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

//~melani
double sim_mob::medium::Driver::updatePositionOnLink(DriverUpdateParams& p) {
	//Determine how far forward we've moved.

	//vehicle->setVelocity(vehicle->getCurrSegment()->maxSpeed * 100 / 3.6 ); //~melani - for now using max speed of segment
	// Fetch density of current road segment and compute speed from speed density function
	double densityOfCurrSegment;
	try{
		densityOfCurrSegment = AuraManager::instance().getDensity(vehicle->getCurrSegment());
	}
	catch (std::exception &e){
		densityOfCurrSegment = 0.0;
	}
	vehicle->setVelocity(speed_density_function(densityOfCurrSegment));
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

	//Build up a list of target lanes.
	nextLaneInNextLink = nullptr;
	vector<const Lane*> targetLanes;
	if (currEndNode && nextSegment) {
		const set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(vehicle->getCurrSegment());
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


//~melani
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
			path = StreetDirectory::instance().shortestDrivingPath(*origin.node, *goal.node);
		} else {
			//Retrieve the special string.
			size_t cInd = parentP->specialStr.find(':');
			string specialType = parentP->specialStr.substr(0, cInd);
			string specialValue = parentP->specialStr.substr(cInd, std::string::npos);
			if (specialType=="loop") {
				initLoopSpecialString(path, specialValue);
			} else if (specialType=="tripchain") {
				path = StreetDirectory::instance().shortestDrivingPath(*origin.node, *goal.node);
				int x = path.size();
				initTripChainSpecialString(specialValue);
			} else {
				throw std::runtime_error("Unknown special string type.");
			}
		}
		//TODO: Start in lane 0?
		int startlaneID = 0;

		// Bus should be at least 1200 to be displayed on Visualizer
		const double length = /*dynamic_cast<BusDriver*>(this) ? 1200 :*/ 400;
		const double width = 200;

		//A non-null vehicle means we are moving.
		if (allocateVehicle) {
			res = new Vehicle(path, startlaneID, length, width);
		}
	}

	//to indicate that the path to next activity is already planned
	//to indicate that the path to next activity is already planned
	parent->setNextPathPlanned(true);
	return res;
}

void sim_mob::medium::Driver::initTripChainSpecialString(const string& value)
{
	//Sanity check
	if (value.length()<=1) {
		throw std::runtime_error("Invalid special tripchain string.");
	}
	Person* p = dynamic_cast<Person*>(parent);
	if (!p) {
		throw std::runtime_error("Parent is not of type Person");
	}
}


void sim_mob::medium::Driver::initLoopSpecialString(vector<WayPoint>& path, const string& value)
{
	//In special cases, we may be manually specifying a loop, e.g., "loop:A:5" in the special string.
	// In this case, "value" will contain ":A:5"; the "loop" having been parsed.
	if (value.size()<=1) {
		throw std::runtime_error("Bad \"loop\" special string.");
	}
	//Repeat this path X times.
	vector<WayPoint> part = LoadSpecialPath(parent->originNode, value[1]);

	size_t ind = value.find(':', 1);
	if (ind != string::npos && ++ind < value.length()) {
		int amount = -1;
		std::istringstream(value.substr(ind, string::npos)) >> amount;

		for (size_t i = 0; static_cast<int> (i) < amount && amount > 1; i++) {
			path.insert(path.end(), part.begin(), part.end());
		}
	} else {
		throw std::runtime_error("Bad \"loop\" special string.");
	}

	//Just in case one of these failed.
	if (path.empty()) {
		path.insert(path.end(), part.begin(), part.end());
	}
}

//This function is associated with the driver class for 2 reasons
// 1. This function is specific to the medium term
// 2. It makes sense in the real life as well that the driver decides to slow down or accelerate based on the traffic density around him
double sim_mob::medium::Driver::speed_density_function(double density){
	/* TODO: min density, jam density, alpha and beta must be obtained from the database.
	 * Since we don't have this data, we have taken the average values from supply parameters of Singapore express ways.
	 * This must be changed after we have this data for each road segment in the database.  */
	double freeFlowSpeed = vehicle->getCurrSegment()->maxSpeed / 3.6 * 100; // Converting from Kmph to cm/s
	double jamDensity = 0.2335; //density during traffic jam
	double alpha = 3.75; //Model parameter of the speed density function
	double beta = 0.5645; //Model parameter of the speed density function
	double minDensity = 0.0048; // minimum traffic density

	//Speed-Density function
	if(density <= minDensity){
		return freeFlowSpeed;
	}
	else {
		return freeFlowSpeed * pow((1 - pow((density - minDensity)/jamDensity, alpha)),beta);
	}
}

