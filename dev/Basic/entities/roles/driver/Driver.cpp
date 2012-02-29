/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Driver.cpp
 *
 *  Created on: 2011-7-5
 *      Author: wangxy & Li Zhemin
 */

#include "Driver.hpp"

#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/Person.hpp"
#include "entities/Signal.hpp"
#include "entities/AuraManager.hpp"
#include "entities/UpdateParams.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/Point2D.hpp"
#include "util/OutputUtil.hpp"
#include "util/DynamicVector.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DebugFlags.hpp"

#include "partitions/PartitionManager.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
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
const int distanceInFront = 2000;
const int distanceBehind = 500;

//Output helper
string PrintLCS(LANE_CHANGE_SIDE s) {
	if (s == LCS_LEFT) {
		return "LCS_LEFT";
	} else if (s == LCS_RIGHT) {
		return "LCS_RIGHT";
	}
	return "LCS_SAME";
}

//used in lane changing, find the start index and end index of polyline in the target lane
size_t updateStartEndIndex(const std::vector<sim_mob::Point2D>* const currLanePolyLine, double currLaneOffset,
		size_t defaultValue) {
	double offset = 0;
	for (size_t i = 0; i < currLanePolyLine->size() - 1; i++) {
		double xOffset = currLanePolyLine->at(i + 1).getX() - currLanePolyLine->at(i).getX();
		double yOffset = currLanePolyLine->at(i + 1).getY() - currLanePolyLine->at(i).getY();
		offset += sqrt(xOffset * xOffset + yOffset * yOffset);
		if (offset >= currLaneOffset) {
			return i;
		}
	}
	return defaultValue;
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

} //End anon namespace


//Initialize
sim_mob::Driver::Driver(Person* parent, MutexStrategy mtxStrat, unsigned int reacTime_LeadingVehicle, unsigned int reacTime_SubjectVehicle,
		unsigned int reacTime_Gap) :
	Role(parent), currLane_(mtxStrat, nullptr), currLaneOffset_(mtxStrat, 0), currLaneLength_(mtxStrat, 0), isInIntersection(mtxStrat, false),
			vehicle(nullptr), params(parent->getGenerator()), perceivedVelocity(reacTime_SubjectVehicle, true), perceivedVelocityOfFwdCar(
					reacTime_LeadingVehicle, true), perceivedAccelerationOfFwdCar(reacTime_LeadingVehicle, true),
			perceivedDistToFwdCar(reacTime_Gap, true),perceivedTrafficSignalStop(0, true) {
	if (Debug::Drivers) {
		DebugStream << "Driver starting: " << parent->getId() << endl;
	}

	trafficSignal = nullptr;

	//Initialize our models. These should be swapable later.
	lcModel = new MITSIM_LC_Model();
	cfModel = new MITSIM_CF_Model();
	intModel = new SimpleIntDrivingModel();

	//Some one-time flags and other related defaults.
	nextLaneInNextLink = nullptr;
}


void sim_mob::Driver::frame_init(UpdateParams& p)
{
	//Save the path from orign to destination in allRoadSegments
	initializePath();

	//Set some properties about the current path, such as the current polyline, etc.
	if (vehicle && vehicle->hasPath()) {
		setOrigin(params);
	} else {
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout << "ERROR: Vehicle could not be created for driver; no route!\n";
#endif
	}
}

//Main update functionality
void sim_mob::Driver::frame_tick(UpdateParams& p)
{
	DriverUpdateParams& p2 = dynamic_cast<DriverUpdateParams&>(p);

	//Are we done already?
	if (vehicle->isDone()) {
		parent->setToBeRemoved();
		return;
	}

	//Just a bit glitchy...
	updateAdjacentLanes(p2);

	//retrieved their current "sensed" values.
	if (perceivedVelocity.can_sense(p.currTimeMS)) {
		p2.perceivedFwdVelocity = perceivedVelocity.sense(p.currTimeMS,0)->x;
		p2.perceivedLatVelocity = perceivedVelocity.sense(p.currTimeMS,0)->y;
	}

	//General update behavior.
	//Note: For now, most updates cannot take place unless there is a Lane and vehicle.
	if (p2.currLane && vehicle) {

		if (update_sensors(p2, p.frameNumber) && update_movement(p2, p.frameNumber) && update_post_movement(p2, p.frameNumber)) {

			//Update parent data. Only works if we're not "done" for a bad reason.
			setParentBufferedData();
		}
	}


	//Update our Buffered types
	//TODO: Update parent buffered properties, or perhaps delegate this.
	//	currLane_.set(params.currLane);
	//	currLaneOffset_.set(params.currLaneOffset);
	//	currLaneLength_.set(params.currLaneLength);
	if (!vehicle->isInIntersection()) {
		currLane_.set(vehicle->getCurrLane());
		currLaneOffset_.set(vehicle->getDistanceMovedInSegment());
		currLaneLength_.set(vehicle->getCurrLinkLaneZeroLength());
	}

	isInIntersection.set(vehicle->isInIntersection());
	//Update your perceptions
	perceivedVelocity.delay(new DPoint(vehicle->getVelocity(), vehicle->getLatVelocity()), p.currTimeMS);
	//Print output for this frame.
}

void sim_mob::Driver::frame_tick_output(const UpdateParams& p)
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
}

void sim_mob::Driver::frame_tick_output_mpi(frame_t frameNumber)
{
	if (frameNumber < parent->getStartTime())
		return;

	if (vehicle->isDone())
		return;

#ifndef SIMMOB_DISABLE_OUTPUT
	double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();
	std::stringstream logout;

	logout << "(\"Driver\"" << "," << frameNumber << "," << parent->getId() << ",{" << "\"xPos\":\""
			<< static_cast<int> (vehicle->getX()) << "\",\"yPos\":\"" << static_cast<int> (vehicle->getY())
			<< "\",\"angle\":\"" << (360 - (baseAngle * 180 / M_PI)) << "\",\"length\":\""
			<< static_cast<int> (vehicle->length) << "\",\"width\":\"" << static_cast<int> (vehicle->width);

	if (this->parent->isFake) {
		logout << "\",\"fake\":\"" << "true";
	} else {
		logout << "\",\"fake\":\"" << "false";
	}

	logout << "\"})" << std::endl;

	LogOut(logout.str());
#endif
}

sim_mob::UpdateParams& sim_mob::Driver::make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS)
{
	params.reset(frameNumber, currTimeMS, *this);
	return params;
}


///Note that Driver's destructor is only for reclaiming memory.
///  If you want to remove its registered properties from the Worker (which you should do!) then
///  this should occur elsewhere.
sim_mob::Driver::~Driver() {
	//Our movement models.
	safe_delete(lcModel);
	safe_delete(cfModel);
	safe_delete(intModel);

	//Our vehicle
	safe_delete(vehicle);
}

vector<BufferedBase*> sim_mob::Driver::getSubscriptionParams() {
	vector<BufferedBase*> res;
	res.push_back(&(currLane_));
	res.push_back(&(currLaneOffset_));
	res.push_back(&(currLaneLength_));
	res.push_back(&(isInIntersection));
	return res;
}

void sim_mob::DriverUpdateParams::reset(frame_t frameNumber, unsigned int currTimeMS, const Driver& owner)
{
	UpdateParams::reset(frameNumber, currTimeMS);

	//Set to the previous known buffered values
	currLane = owner.currLane_.get();
	currLaneIndex = getLaneIndex(currLane);
	currLaneLength = owner.currLaneLength_.get();
	currLaneOffset = owner.currLaneOffset_.get();

	//Current lanes to the left and right. May be null
	leftLane = nullptr;
	rightLane = nullptr;

	//Reset; these will be set before they are used; the values here represent either defaul
	//       values or are unimportant.
	currSpeed = 0;
	perceivedFwdVelocity = 0;
	perceivedLatVelocity = 0;
	isTrafficLightStop = false;
	trafficSignalStopDistance = 5000;
	elapsedSeconds = ConfigParams::GetInstance().baseGranMS / 1000.0;

	//Lateral velocity of lane changing.
	laneChangingVelocity = 100;

	//Are we near a crossing?
	isCrossingAhead = false;

	//relative x coordinate for crossing, the intersection point of crossing's front line and current polyline
	crossingFwdDistance = 0;

	//Space to next car
	space = 0;

	//the acceleration of leading vehicle
	a_lead = 0;

	//the speed of leading vehicle
	v_lead = 0;

	//the distance which leading vehicle will move in next time step
	space_star = 0;

	distanceToNormalStop = 0;

	//distance to where critical location where lane changing has to be made
	dis2stop = 0;

	//in MLC: is the vehicle waiting acceptable gap to change lane
	isWaiting = false; //TODO: This might need to be saved between turns.

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


bool sim_mob::Driver::update_sensors(DriverUpdateParams& params, frame_t frameNumber) {
	//Are we done?
	if (vehicle->isDone()) {
		return false;
	}

	//Save the nearest agents in your lane and the surrounding lanes, stored by their
	// position before/behind you. Save nearest fwd pedestrian too.

	//Manage traffic signal behavior if we are close to the end of the link.
	//TODO: This might be slightly inaccurate if a vehicle leaves an intersection
	//      on a particularly short road segment. For now, though, I'm just organizing these
	//      functions with structure in mind, and it won't affect our current network.
	params.isApproachingToIntersection = false;
	if (!vehicle->getNextSegment() && !vehicle->isInIntersection()) {
		params.isApproachingToIntersection = true;
		setTrafficSignalParams(params);
	}

	updateNearbyAgents(params);


	return true;
}

bool sim_mob::Driver::update_movement(DriverUpdateParams& params, frame_t frameNumber) {
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
	//TODO: LastKnownPolypoint should actually be the _new_ polypoint.

	const RoadSegment* prevSegment = vehicle->getCurrSegment();

	params.TEMP_lastKnownPolypoint = DPoint(vehicle->getCurrPolylineVector().getEndX(),
			vehicle->getCurrPolylineVector().getEndY());


	//First, handle driving behavior inside an intersection.
	if (vehicle->isInIntersection()) {
		intersectionDriving(params);
	}

	//Next, handle driving on links.
	// Note that a vehicle may leave an intersection during intersectionDriving(), so the conditional check is necessary.
	// Note that there is no need to chain this back to intersectionDriving.
	if (!vehicle->isInIntersection()) {

		params.overflowIntoIntersection = linkDriving(params);

		//Did our last move forward bring us into an intersection?
		if (vehicle->isInIntersection()) {
			params.justMovedIntoIntersection = true;
			vehicle->setLatVelocity(0);
			vehicle->setTurningDirection(LCS_SAME);
		}
	}


	//Has the segment changed?
	if (!vehicle->isDone()) {
		params.justChangedToNewSegment = (vehicle->getCurrSegment() != prevSegment);
	}
	return true;
}

bool sim_mob::Driver::update_post_movement(DriverUpdateParams& params, frame_t frameNumber) {
	//Are we done?
	if (vehicle->isDone()) {
		return false;
	}

	//Has the segment changed?
	if (!vehicle->isInIntersection() && params.justChangedToNewSegment) {
		//Make pre-intersection decisions?
		if (!vehicle->hasNextSegment(true))
			saveCurrTrafficSignal();
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


//responsible for vehicle behaviour inside intersection
//the movement is based on absolute position
void sim_mob::Driver::intersectionDriving(DriverUpdateParams& p) {
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

bool sim_mob::Driver::AvoidCrashWhenLaneChanging(DriverUpdateParams& p)
{
	double distanceRange =500; //currently, set 5m as the range which maybe cause two vehicles crash
	//the subject vehicle isn't doing lane changing
	if(vehicle->getLatVelocity()==0)
		return false;
	//the subject vehicle is changing to left lane
	if(vehicle->getLatVelocity()>0 && p.nvLeftFwd2.exists() && p.nvLeftFwd2.distance < distanceRange
			&& p.nvLeftFwd2.driver->getVehicle()->getLatVelocity()<0)
		return true;
	if(vehicle->getLatVelocity()<0 && p.nvRightFwd2.exists() && p.nvRightFwd2.distance < distanceRange
			&& p.nvRightFwd2.driver->getVehicle()->getLatVelocity()>0)
		return true;
	return false;
}

//vehicle movement on link, perform acceleration, lane changing if necessary
//the movement is based on relative position
double sim_mob::Driver::linkDriving(DriverUpdateParams& p) {


	if (!vehicle->hasNextSegment(true)) {
		p.dis2stop = vehicle->getAllRestRoadSegmentsLength() - vehicle->getDistanceMovedInSegment() - vehicle->length
				/ 2 - 300;
		if (p.nvFwd.distance < p.dis2stop)
			p.dis2stop = p.nvFwd.distance;
		p.dis2stop /= 100;
	} else
		p.dis2stop = 1000;//defalut 1000m

	if (p.fromLaneIndex >= p.currLane->getRoadSegment()->getLanes().size())
		p.fromLaneIndex = p.currLaneIndex;
	//Check if we should change lanes.
	double newLatVel = lcModel->executeLaneChanging(p, vehicle->getAllRestRoadSegmentsLength(), vehicle->length,
			vehicle->getTurningDirection());

	vehicle->setLatVelocity(newLatVel);
	if(vehicle->getLatVelocity()>0)
		vehicle->setTurningDirection(LCS_LEFT);
	else if(vehicle->getLatVelocity()<0)
		vehicle->setTurningDirection(LCS_RIGHT);
	else
		vehicle->setTurningDirection(LCS_SAME);
	//when vehicle stops, don't do lane changing
//	if (vehicle->getVelocity() <= 0) {
//		vehicle->setLatVelocity(0);
//	}

	//get nearest car, if not making lane changing, the nearest car should be the leading car in current lane.
	//if making lane changing, adjacent car need to be taken into account.
	NearestVehicle & nv = nearestVehicle(p);
	if(nv.distance<=0)
	{
		if(nv.driver->parent->getId() > parent->getId())
		{
			nv = NearestVehicle();
		}
	}

	perceivedDataProcess(nv, p);


	//Retrieve a new acceleration value.
	double newFwdAcc = 0;

	//Convert back to m/s
	//TODO: Is this always m/s? We should rename the variable then...
	p.currSpeed = vehicle->getVelocity() / 100;
	//Call our model
	newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, maxLaneSpeed);

	//Update our chosen acceleration; update our position on the link.
	vehicle->setAcceleration(newFwdAcc * 100);
	if(AvoidCrashWhenLaneChanging(p))
	{
		vehicle->setLatVelocity(0);
		vehicle->setAcceleration(0);
	}

	return updatePositionOnLink(p);
}

//for buffer data, maybe need more parameters to be stored
//TODO: need to discuss
void sim_mob::Driver::setParentBufferedData() {
	parent->xPos.set(vehicle->getX());
	parent->yPos.set(vehicle->getY());

	//TODO: Need to see how the parent agent uses its velocity vector.
	parent->fwdVel.set(vehicle->getVelocity());
	parent->latVel.set(vehicle->getLatVelocity());
}

namespace {
vector<const Agent*> GetAgentsInCrossing(const Crossing* crossing) {
	//Put x and y coordinates into planar arrays.
	int x[4] = { crossing->farLine.first.getX(), crossing->farLine.second.getX(), crossing->nearLine.first.getX(),
			crossing->nearLine.second.getX() };
	int y[4] = { crossing->farLine.first.getY(), crossing->farLine.second.getY(), crossing->nearLine.first.getY(),
			crossing->nearLine.second.getY() };

	//Prepare minimum/maximum values.
	int xmin = x[0];
	int xmax = x[0];
	int ymin = y[0];
	int ymax = y[0];
	for (int i = 0; i < 4; i++) {
		if (x[i] < xmin)
			xmin = x[i];
		if (x[i] > xmax)
			xmax = x[i];
		if (y[i] < ymin)
			ymin = y[i];
		if (y[i] > ymax)
			ymax = y[i];
	}

	//Create a rectangle from xmin,ymin to xmax,ymax
	//TODO: This is completely unnecessary; Crossings are already in order, so
	//      crossing.far.first and crossing.near.second already defines a rectangle.
	Point2D rectMinPoint = Point2D(xmin, ymin);
	Point2D rectMaxPoint = Point2D(xmax, ymax);

	return AuraManager::instance().agentsInRect(rectMinPoint, rectMaxPoint);
}
} //End anon namespace


bool sim_mob::Driver::isPedestrianOnTargetCrossing() const {
	if (!trafficSignal) {
		return false;
	}

	map<Link const*, size_t> const linkMap = trafficSignal->links_map();
	int index = -1;
	for (map<Link const*, size_t>::const_iterator link_i = linkMap.begin(); link_i != linkMap.end(); link_i++) {
		if (vehicle->getNextSegment() && link_i->first == vehicle->getNextSegment()->getLink()) {
			index = (*link_i).second;
			break;
		}
	}

	map<Crossing const *, size_t> const crossingMap = trafficSignal->crossings_map();
	const Crossing* crossing = nullptr;
	for (map<Crossing const *, size_t>::const_iterator crossing_i = crossingMap.begin(); crossing_i
			!= crossingMap.end(); crossing_i++) {
		if (static_cast<int> (crossing_i->second) == index) {
			crossing = crossing_i->first;
			break;
		}
	}

	//Have we found a relevant crossing?
	if (!crossing) {
		return false;
	}

	//Search through the list of agents in that crossing.
	vector<const Agent*> agentsInRect = GetAgentsInCrossing(crossing);
	for (vector<const Agent*>::iterator it = agentsInRect.begin(); it != agentsInRect.end(); it++) {
		const Person* other = dynamic_cast<const Person *> (*it);
		if (other) {
			const Pedestrian* pedestrian = dynamic_cast<const Pedestrian*> (other->getRole());
			if (pedestrian && pedestrian->isOnCrossing()) {
				return true;
			}
		}
	}
	return false;
}

//calculate current lane length
/*void sim_mob::Driver::updateCurrLaneLength(DriverUpdateParams& p)
 {
 p.currLaneLength = vehicle->getCurrPolylineVector().getMagnitude();
 }*/

//update left and right lanes of the current lane
//if there is no left or right lane, it will be null
void sim_mob::Driver::updateAdjacentLanes(DriverUpdateParams& p) {
	//Need to reset, we can call this after DriverUpdateParams is initialized.
	p.leftLane = nullptr;
	p.rightLane = nullptr;
	p.leftLane2 = nullptr;
	p.rightLane2 = nullptr;

	if (!p.currLane) {
		return; //Can't do anything without a lane to reference.
	}

	if (p.currLaneIndex > 0) {
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex - 1);
		if(!temp->is_pedestrian_lane())
			p.rightLane = temp;
	}
	if (p.currLaneIndex > 1) {
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex - 2);
		if(!temp->is_pedestrian_lane())
			p.rightLane2 = temp;
	}

	if (p.currLaneIndex < p.currLane->getRoadSegment()->getLanes().size() - 1) {
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex + 1);
		if(!temp->is_pedestrian_lane())
			p.leftLane = temp;
	}

	if (p.currLaneIndex < p.currLane->getRoadSegment()->getLanes().size() - 2) {
		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex + 2);
		if(!temp->is_pedestrian_lane())
			p.leftLane2 = temp;
	}
}

//General update information for whenever a Segment may have changed.
void sim_mob::Driver::syncCurrLaneCachedInfo(DriverUpdateParams& p) {
	//The lane may have changed; reset the current lane index.
	p.currLaneIndex = getLaneIndex(p.currLane);

	//Update which lanes are adjacent.
	updateAdjacentLanes(p);

	//Update the length of the current road segment.
	p.currLaneLength = vehicle->getCurrLinkLaneZeroLength();

	//Finally, update target/max speed to match the new Lane's rules.
	maxLaneSpeed = vehicle->getCurrSegment()->maxSpeed / 3.6; //slow down
	targetSpeed = maxLaneSpeed;
}

//currently it just chooses the first lane from the targetLane
//Note that this also sets the target lane so that we (hopefully) merge before the intersection.
void sim_mob::Driver::chooseNextLaneForNextLink(DriverUpdateParams& p) {
	p.fromLaneIndex = p.currLaneIndex;
	//Retrieve the node we're on, and determine if this is in the forward direction.
	const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (vehicle->getNodeMovingTowards());
	const RoadSegment* nextSegment = vehicle->getNextSegment(false);

	//Build up a list of target lanes.
	nextLaneInNextLink = nullptr;
	vector<const Lane*> targetLanes;
	if (currEndNode && nextSegment) {
		const set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(*vehicle->getCurrSegment());
		for (set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++) {
			if ((*it)->getLaneTo()->getRoadSegment() == nextSegment && (*it)->getLaneFrom() == p.currLane) {
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

//TODO: For now, we're just using a simple trajectory model. Complex curves may be added later.
void sim_mob::Driver::calculateIntersectionTrajectory(DPoint movingFrom, double overflow) {
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

//link path should be retrieved from other class
//for now, it serves as this purpose
void sim_mob::Driver::initializePath() {
	//Save local copies of the parent's origin/destination nodes.
	origin.node = parent->originNode;
	origin.point = origin.node->location;
	goal.node = parent->destNode;
	goal.point = goal.node->location;

	//TEMP
	std::stringstream errorMsg;

	//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
	vector<WayPoint> path;
	Person* parentP = dynamic_cast<Person*> (parent);
	errorMsg << "Attempting to generate a vehicle" << std::endl;
	if (!parentP || parentP->specialStr.empty()) {
		path = StreetDirectory::instance().shortestDrivingPath(*origin.node, *goal.node);

		errorMsg << "...from node: " << origin.node->originalDB_ID.getLogItem() << " to node: "
				<< goal.node->originalDB_ID.getLogItem() << std::endl;
	} else {
		errorMsg << "...special path." << std::endl;

		//In special cases, we may be manually specifying a loop, e.g., "loop:A:5" in the special string.
		size_t cInd = parentP->specialStr.find(':');
		if (cInd != string::npos && cInd + 1 < parentP->specialStr.length()) {
			//Repeat this path X times.
			vector<WayPoint> part = LoadSpecialPath(parent->originNode, parentP->specialStr[cInd + 1]);

			cInd = parentP->specialStr.find(':', cInd + 1);
			if (cInd != string::npos && cInd + 1 < parentP->specialStr.length()) {
				int amount = -1;
				std::istringstream(parentP->specialStr.substr(cInd + 1, string::npos)) >> amount;

				for (size_t i = 0; static_cast<int> (i) < amount && amount > 1; i++) {
					path.insert(path.end(), part.begin(), part.end());
				}
			}

			//Just in case one of these failed.
			if (path.empty()) {
				path = part;
			}
		}
	}

	//A non-null vehicle means we are moving.
	//TODO: Start in lane 0?
	try {
		//vehicle length and width
		double length = 400;
		double width = 200;
//		size_t type = parent->getId()%10;
//		if(type==0)//bus
//			length = 1200;
//		else if(type==1||type==2)//truck
//			length = 1500;
//		else//car
//			length = 500;
		int startlaneID = 0;
//		if(parent->getId()%2==0)
//			startlaneID = 0;
//		else
//			startlaneID = 2;
		vehicle = new Vehicle(path, startlaneID, length, width);
	} catch (std::exception& ex) {
		errorMsg << "ERROR: " << ex.what();
		std::cout << errorMsg.str() << std::endl;
		throw ex;
	}
}

void sim_mob::Driver::setOrigin(DriverUpdateParams& p) {
	//Set the max speed and target speed.
	maxLaneSpeed = vehicle->getCurrSegment()->maxSpeed / 3.6;//slow down
	targetSpeed = maxLaneSpeed;

	//Set our current and target lanes.
	p.currLane = vehicle->getCurrLane();
	p.currLaneIndex = getLaneIndex(p.currLane);
	targetLaneIndex = p.currLaneIndex;

	//Vehicles start at rest
	vehicle->setVelocity(0);
	vehicle->setLatVelocity(0);
	vehicle->setAcceleration(0);

	//Scan and save the lanes to the left and right.
	//updateAdjacentLanes(p);

	//Calculate and save the total length of the current polyline.
	p.currLaneLength = vehicle->getCurrLinkLaneZeroLength();

	//if the first road segment is the last one in this link
	if (!vehicle->hasNextSegment(true)) {
		saveCurrTrafficSignal();
	}
	if (!vehicle->hasNextSegment(true) && vehicle->hasNextSegment(false)) {
		//Don't do this if there is no next link.
		chooseNextLaneForNextLink(p);
	}
}

//TODO
void sim_mob::Driver::findCrossing(DriverUpdateParams& p) {
	const Crossing* crossing = dynamic_cast<const Crossing*> (vehicle->getCurrSegment()->nextObstacle(
			vehicle->getDistanceMovedInSegment(), true).item);

	if (crossing) {
		//TODO: Please double-check that this does what's intended.
		Point2D interSect = LineLineIntersect(vehicle->getCurrPolylineVector(), crossing->farLine.first,
				crossing->farLine.second);
		DynamicVector toCrossing(vehicle->getX(), vehicle->getY(), interSect.getX(), interSect.getY());

		p.crossingFwdDistance = toCrossing.getMagnitude();
		p.isCrossingAhead = true;
	}
}

double sim_mob::Driver::updatePositionOnLink(DriverUpdateParams& p) {
	//Determine how far forward we've moved.
	//TODO: I've disabled the acceleration component because it doesn't really make sense.
	//      Please re-enable if you think this is expected behavior. ~Seth
//	if(parent->getId() == 8)
//	LogOut("8,77\n");

	double fwdDistance = vehicle->getVelocity() * p.elapsedSeconds + 0.5 * vehicle->getAcceleration()
			* p.elapsedSeconds * p.elapsedSeconds;
	if (fwdDistance < 0)
		fwdDistance = 0;
	//double fwdDistance = vehicle->getVelocity()*p.elapsedSeconds;
	double latDistance = vehicle->getLatVelocity() * p.elapsedSeconds;

	//Increase the vehicle's velocity based on its acceleration.
	vehicle->setVelocity(vehicle->getVelocity() + vehicle->getAcceleration() * p.elapsedSeconds);

	//TEMP: For ns3
	Person* parentP = dynamic_cast<Person*> (parent);
	if (parentP && !parentP->specialStr.empty() && parentP->specialStr.at(5) == 'A') {
		vehicle->setVelocity(vehicle->getVelocity() * 1.1);
	}

	//when v_lead and a_lead is 0, space is not negative, the Car Following will generate an acceleration based on free flowing model
	//this causes problem, so i manually set acceleration and velocity to 0
	if (vehicle->getVelocity() < 0 ||(p.space<1&&p.v_lead==0&&p.a_lead==0)) {
		//Set to 0 forward velocity, no acceleration.
		vehicle->setVelocity(0.0);
		vehicle->setAcceleration(0);
	}

	//Move the vehicle forward.
	double res = 0.0;
	try {
//		if(parent->getId() == 8)
//		LogOut("8,88\n");

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

	//Retrieve what direction we're moving in, since it will "flip" if we cross the relative X axis.
	LANE_CHANGE_SIDE relative = getCurrLaneSideRelativeToCenter();

	//Lateral movement
	if (latDistance != 0) {
		vehicle->moveLat(latDistance);
		updatePositionDuringLaneChange(p, relative);
	}

	//Update our offset in the current lane.
	if (!vehicle->isInIntersection()) {
		p.currLaneOffset = vehicle->getDistanceMovedInSegment();
	}
	return res;
}

//Helper function: check if a modified distance is less than the current minimum and save it.
void sim_mob::Driver::check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh,
		const Driver* other) {
	//Subtract the size of the car from the distance between them
	distance = fabs(distance) - veh->length / 2 - other->getVehicleLength() / 2;
	if (distance <= res.distance) {
		res.driver = other;
		res.distance = distance;
	}

}

//TODO: I have the feeling that this process of detecting nearby drivers in front of/behind you and saving them to
//      the various CFD/CBD/LFD/LBD variables can be generalized somewhat. I shortened it a little and added a
//      helper function; perhaps more cleanup can be done later? ~Seth
void sim_mob::Driver::updateNearbyDriver(DriverUpdateParams& params, const Person* other, const Driver* other_driver) {
	//Only update if passed a valid pointer which is not a pointer back to you, and
	//the driver is not actually in an intersection at the moment.


	if (!(other_driver && this != other_driver && !other_driver->isInIntersection.get())) {
		return;
	}



	//Retrieve the other driver's lane, road segment, and lane offset.
	const Lane* other_lane = other_driver->currLane_.get();
	if (!other_lane) {
		return;
	}
	const RoadSegment* otherRoadSegment = other_lane->getRoadSegment();
	int other_offset = other_driver->currLaneOffset_.get();

	//If the vehicle is in the same Road segment
	if (vehicle->getCurrSegment() == otherRoadSegment) {
		//Set distance equal to the _forward_ distance between these two vehicles.
		int distance = other_offset - params.currLaneOffset;
		bool fwd = distance >= 0;

		//Set different variables depending on where the car is.
		if (other_lane == params.currLane) {//the vehicle is on the current lane
			check_and_set_min_car_dist((fwd ? params.nvFwd : params.nvBack), distance, vehicle, other_driver);
		} else if (other_lane == params.leftLane) { //the vehicle is on the left lane
			check_and_set_min_car_dist((fwd ? params.nvLeftFwd : params.nvLeftBack), distance, vehicle, other_driver);
		} else if (other_lane == params.rightLane) { //The vehicle is on the right lane
			check_and_set_min_car_dist((fwd ? params.nvRightFwd : params.nvRightBack), distance, vehicle,other_driver);
		} else if (other_lane == params.leftLane2) { //The vehicle is on the second Left lane
			check_and_set_min_car_dist((fwd ? params.nvLeftFwd2 : params.nvLeftBack2), distance, vehicle,other_driver);
		} else if (other_lane == params.rightLane2) { //The vehicle is on the second right lane
			check_and_set_min_car_dist((fwd ? params.nvRightFwd2 : params.nvRightBack2), distance, vehicle,other_driver);
		}

	} else if (otherRoadSegment->getLink() == vehicle->getCurrLink()) { //We are in the same link.
		if (vehicle->getNextSegment() == otherRoadSegment) { //Vehicle is on the next segment.
			//Retrieve the next node we are moving to, cast it to a UniNode.
			const Node* nextNode = vehicle->getNodeMovingTowards();
			const UniNode* uNode = dynamic_cast<const UniNode*> (nextNode);

			//Initialize some lane pointers
			const Lane* nextLane = nullptr;
			const Lane* nextLeftLane = nullptr;
			const Lane* nextRightLane = nullptr;
			const Lane* nextLeftLane2 = nullptr;
			const Lane* nextRightLane2 = nullptr;
			if (uNode) {
				nextLane = uNode->getOutgoingLane(*params.currLane);
			}

			//Make sure next lane exists and is in the next road segment, although it should be true
			if (nextLane && nextLane->getRoadSegment() == otherRoadSegment) {
				//Assign next left/right lane based on lane ID.
				size_t nextLaneIndex = getLaneIndex(nextLane);
				if (nextLaneIndex > 0) {
					nextLeftLane = otherRoadSegment->getLanes().at(nextLaneIndex - 1);
				}
				if (nextLaneIndex < otherRoadSegment->getLanes().size() - 1) {
					nextRightLane = otherRoadSegment->getLanes().at(nextLaneIndex + 1);
				}
				if (nextLaneIndex > 1)
				{
					nextLeftLane2 = otherRoadSegment->getLanes().at(nextLaneIndex - 2);
				}
				if (nextLaneIndex < otherRoadSegment->getLanes().size() - 2) {
				    nextRightLane2 = otherRoadSegment->getLanes().at(nextLaneIndex + 2);
				}
			}

			//Modified distance.
			int distance = other_offset + params.currLaneLength - params.currLaneOffset;

			//Set different variables depending on where the car is.
			if (other_lane == nextLane) { //The vehicle is on the current lane
				check_and_set_min_car_dist(params.nvFwd, distance, vehicle, other_driver);
			} else if (other_lane == nextLeftLane) { //the vehicle is on the left lane
				check_and_set_min_car_dist(params.nvLeftFwd, distance, vehicle, other_driver);
			} else if (other_lane == nextRightLane) { //the vehicle is in front
				check_and_set_min_car_dist(params.nvRightFwd, distance, vehicle, other_driver);
			} else if (other_lane == nextLeftLane2) { //The vehicle is on the second Left lane
				check_and_set_min_car_dist(params.nvLeftFwd2, distance, vehicle,other_driver);
			} else if (other_lane == nextRightLane2) { //The vehicle is on the second right lane
				check_and_set_min_car_dist(params.nvRightFwd2, distance, vehicle,other_driver);
			}
		} else if (vehicle->getPrevSegment() == otherRoadSegment) { //Vehicle is on the previous segment.
			//Retrieve the previous node as a UniNode.
			const Node* prevNode = vehicle->getNodeMovingFrom();
			const UniNode* uNode = dynamic_cast<const UniNode*> (prevNode);

			//Set some lane pointers.
			const Lane* preLane = nullptr;
			const Lane* preLeftLane = nullptr;
			const Lane* preRightLane = nullptr;
			const Lane* preLeftLane2 = nullptr;
			const Lane* preRightLane2 = nullptr;

			//Find the node which leads to this one from the UniNode. (Requires some searching; should probably
			//   migrate this to the UniNode class later).
			const vector<Lane*>& lanes = otherRoadSegment->getLanes();
			if (uNode) {
				for (vector<Lane*>::const_iterator it = lanes.begin(); it != lanes.end() && !preLane; it++) {
					if (uNode->getOutgoingLane(**it) == params.currLane) {
						preLane = *it;
					}
				}
			}

			//Make sure next lane is in the next road segment, although it should be true
			if (preLane) {
				//Save the new left/right lanes
				size_t preLaneIndex = getLaneIndex(preLane);
				if (preLaneIndex > 0) {
					preLeftLane = otherRoadSegment->getLanes().at(preLaneIndex - 1);
				}
				if (preLaneIndex < otherRoadSegment->getLanes().size() - 1) {
					preRightLane = otherRoadSegment->getLanes().at(preLaneIndex + 1);
				}
			}

			//Modified distance.
			int distance = other_driver->currLaneLength_.get() - other_offset + params.currLaneOffset;

			//Set different variables depending on where the car is.
			if (other_lane == preLane) { //The vehicle is on the current lane
				check_and_set_min_car_dist(params.nvBack, distance, vehicle, other_driver);
			} else if (other_lane == preLeftLane) { //the vehicle is on the left lane
				check_and_set_min_car_dist(params.nvLeftBack, distance, vehicle, other_driver);
			} else if (other_lane == preRightLane) { //the vehicle is on the right lane
				check_and_set_min_car_dist(params.nvRightBack, distance, vehicle, other_driver);
			} else if (other_lane == preLeftLane2) { //The vehicle is on the second Left lane
				check_and_set_min_car_dist(params.nvLeftBack2, distance, vehicle,other_driver);
			} else if (other_lane == preRightLane2) { //The vehicle is on the second right lane
				check_and_set_min_car_dist(params.nvRightBack2, distance, vehicle,other_driver);
			}
		}
	}

}

void sim_mob::Driver::updateNearbyPedestrian(DriverUpdateParams& params, const Person* other, const Pedestrian* pedestrian) {
	//Only update if passed a valid pointer and this is on a crossing.

	if (!(pedestrian && pedestrian->isOnCrossing())) {
		return;
	}

	//TODO: We are using a vector to check the angle to the Pedestrian. There are other ways of doing this which may be more accurate.
	const std::vector<sim_mob::Point2D>& polyLine = vehicle->getCurrSegment()->getLanes().front()->getPolyline();
	DynamicVector otherVect(polyLine.front().getX(), polyLine.front().getY(), other->xPos.get(), other->yPos.get());

	//Calculate the distance between these two vehicles and the distance between the angle of the
	// car's forward movement and the pedestrian.
	//NOTE: I am changing this slightly, since cars were stopping for pedestrians on the opposite side of
	//      the road for no reason (traffic light was green). ~Seth
	//double distance = otherVect.getMagnitude();
	double angleDiff = 0.0;
	{
		//Retrieve
		DynamicVector fwdVector(vehicle->getCurrPolylineVector());
		fwdVector.scaleVectTo(100);

		//Calculate the difference
		//NOTE: I may be over-complicating this... we can probably use the dot product but that can be done later. ~Seth
		double angle1 = atan2(fwdVector.getEndY() - fwdVector.getY(), fwdVector.getEndX() - fwdVector.getX());
		double angle2 = atan2(otherVect.getEndY() - otherVect.getY(), otherVect.getEndX() - otherVect.getX());
		double diff = fabs(angle1 - angle2);
		angleDiff = std::min(diff, fabs(diff - 2 * M_PI));
	}

	//If the pedestrian is not behind us, then set our flag to true and update the minimum pedestrian distance.
	if (angleDiff < 0.5236) { //30 degrees +/-
		params.npedFwd.distance = std::min(params.npedFwd.distance, otherVect.getMagnitude() - vehicle->length / 2
				- 300);
	}
}

void sim_mob::Driver::updateNearbyAgents(DriverUpdateParams& params) {
	//Retrieve a list of nearby agents
	vector<const Agent*> nearby_agents = AuraManager::instance().nearbyAgents(
			Point2D(vehicle->getX(), vehicle->getY()), *params.currLane, distanceInFront, distanceBehind);

	//Update each nearby Pedestrian/Driver
	params.nvFwd.distance = 5000;
	params.nvFwd = NearestVehicle();
	params.nvLeftFwd = NearestVehicle();
	params.nvRightFwd = NearestVehicle();
	params.nvBack = NearestVehicle();
	params.nvLeftBack = NearestVehicle();
	params.nvRightBack = NearestVehicle();
	params.nvLeftFwd2 = NearestVehicle();
	params.nvLeftBack2 = NearestVehicle();
	params.nvRightFwd2 = NearestVehicle();
	params.nvRightBack2 = NearestVehicle();

	for (vector<const Agent*>::iterator it = nearby_agents.begin(); it != nearby_agents.end(); it++) {
		//Perform no action on non-Persons
		const Person* other = dynamic_cast<const Person *> (*it);
		if (!other) {
			continue;
		}

		//Perform a different action depending on whether or not this is a Pedestrian/Driver/etc.
		updateNearbyDriver(params, other, dynamic_cast<const Driver*> (other->getRole()));
		updateNearbyPedestrian(params, other, dynamic_cast<const Pedestrian*> (other->getRole()));
	}
}

void sim_mob::Driver::perceivedDataProcess(NearestVehicle & nv, DriverUpdateParams& params)
{
	//Update your perceptions for leading vehicle and gap
	perceivedDistToFwdCar.delay(nv.distance, params.currTimeMS);
	if (params.nvFwd.distance != 5000) {
		perceivedVelocityOfFwdCar.delay(new DPoint(nv.driver->getVehicle()->getVelocity(),
				nv.driver->getVehicle()->getLatVelocity()), params.currTimeMS);
		perceivedAccelerationOfFwdCar.delay(nv.driver->getVehicle()->getAcceleration(), params.currTimeMS);

		//retrieve perceptions
		size_t delayMS = 1500;
		//make delay time changeable
		if(params.isApproachingToIntersection&&!params.isTrafficLightStop)
		{
			int numV = (vehicle->getAllRestRoadSegmentsLength()
					- vehicle->getDistanceMovedInSegment())/vehicle->length;
			if(numV<=0)
				numV=1;
			delayMS = delayMS/numV;
		}
		if (perceivedVelocityOfFwdCar.can_sense(params.currTimeMS)
				&&perceivedAccelerationOfFwdCar.can_sense(params.currTimeMS)
				&&perceivedDistToFwdCar.can_sense(params.currTimeMS)) {
			params.perceivedFwdVelocityOfFwdCar = perceivedVelocityOfFwdCar.sense(params.currTimeMS,delayMS)->x;
			params.perceivedLatVelocityOfFwdCar = perceivedVelocityOfFwdCar.sense(params.currTimeMS,delayMS)->y;
			params.perceivedAccelerationOfFwdCar = perceivedAccelerationOfFwdCar.sense(params.currTimeMS,delayMS);
			params.perceivedDistToFwdCar = perceivedDistToFwdCar.sense(params.currTimeMS,delayMS);
		}
		else
		{
			params.perceivedFwdVelocityOfFwdCar = params.nvFwd.driver->getVehicle()->getVelocity();
			params.perceivedLatVelocityOfFwdCar = params.nvFwd.driver->getVehicle()->getLatVelocity();
			params.perceivedAccelerationOfFwdCar = params.nvFwd.driver->getVehicle()->getAcceleration();
			params.perceivedDistToFwdCar = params.nvFwd.distance;
		}
	}

}

NearestVehicle & sim_mob::Driver::nearestVehicle(DriverUpdateParams& p)
{
	double leftDis = p.nvLeftFwd.distance;
	double rightDis = p.nvRightFwd.distance;
	double currentDis = p.nvFwd.distance;
	if(leftDis<currentDis)
	{
		//the vehicle in the left lane is turning to right
		//or subject vehicle is turning to left
		if(p.nvLeftFwd.driver->getVehicle()->getTurningDirection()==LCS_RIGHT ||
				vehicle->getTurningDirection()==LCS_LEFT)
			return p.nvLeftFwd;
	}
	else if(rightDis<currentDis)
	{
		if(p.nvRightFwd.driver->getVehicle()->getTurningDirection()==LCS_LEFT ||
				vehicle->getTurningDirection()==LCS_RIGHT)
			return p.nvRightFwd;
	}

	return p.nvFwd;
}

void sim_mob::Driver::intersectionVelocityUpdate() {
	double inter_speed = 1000;//10m/s
	vehicle->setAcceleration(0);

	//Set velocity for intersection movement.
	vehicle->setVelocity(inter_speed);
}

void sim_mob::Driver::justLeftIntersection(DriverUpdateParams& p) {
	p.currLane = nextLaneInNextLink;
	p.currLaneIndex = getLaneIndex(p.currLane);
	vehicle->moveToNewLanePolyline(p.currLaneIndex);
	syncCurrLaneCachedInfo(p);
	p.currLaneOffset = vehicle->getDistanceMovedInSegment();
	targetLaneIndex = p.currLaneIndex;

	//Reset lateral movement/velocity to zero.
	vehicle->setLatVelocity(0);
	vehicle->resetLateralMovement();
}

LANE_CHANGE_SIDE sim_mob::Driver::getCurrLaneChangeDirection() const {
	if (vehicle->getLatVelocity() > 0) {
		return LCS_LEFT;
	} else if (vehicle->getLatVelocity() < 0) {
		return LCS_RIGHT;
	}
	return LCS_SAME;
}

LANE_CHANGE_SIDE sim_mob::Driver::getCurrLaneSideRelativeToCenter() const {
	if (vehicle->getLateralMovement() > 0) {
		return LCS_LEFT;
	} else if (vehicle->getLateralMovement() < 0) {
		return LCS_RIGHT;
	}
	return LCS_SAME;
}

//TODO: I think all lane changing occurs after 150m. Double-check please. ~Seth
void sim_mob::Driver::updatePositionDuringLaneChange(DriverUpdateParams& p, LANE_CHANGE_SIDE relative) {

	double halfLaneWidth = p.currLane->getWidth() / 2.0;

	//The direction we are attempting to change lanes in
	LANE_CHANGE_SIDE actual = vehicle->getTurningDirection();
	//LANE_CHANGE_SIDE relative = getCurrLaneSideRelativeToCenter();
	if (actual == LCS_SAME) {
		if (Debug::Drivers) {
			DebugStream << "  Lane change: " << "ERROR: Called with \"same\"" << endl;
		}
		return; //Not actually changing lanes.
	}
	if (relative == LCS_SAME) {
		relative = actual; //Unlikely to occur, but we can still work off this.
	}
	if (actual == relative) { //We haven't merged halfway yet; check there's actually a lane for us to merge into.
		if ((actual == LCS_LEFT && !p.leftLane) || (actual == LCS_RIGHT && !p.rightLane)) {
			std::stringstream msg;
			msg <<"Agent (" <<parent->getId() <<") is attempting to merge into a lane that doesn't exist.";
			throw std::runtime_error(msg.str().c_str());
			//return; //Error condition
		}
	}

	if (Debug::Drivers) {
		DebugStream << "  Lane change: " << PrintLCS(actual) << " (" << PrintLCS(relative) << ")" << endl;
	}

	//Basically, we move "halfway" into the next lane, and then move "halfway" back to its midpoint.
	if (actual == relative) {
		//Moving "out".
//		if(parent->getId() == 8)
//		LogOut("8,33\n");

		double remainder = fabs(vehicle->getLateralMovement()) - halfLaneWidth;

		if (Debug::Drivers) {
			DebugStream << "    Moving out on Lane " << p.currLaneIndex << ": " << remainder << endl;
		}

		if (remainder >= 0) {
			//Update Lanes, polylines, RoadSegments, etc.
			p.currLane = (actual == LCS_LEFT ? p.leftLane : p.rightLane);
			syncCurrLaneCachedInfo(p);

//			if(parent->getId() == 8)
//			LogOut("8,55\n");

			vehicle->shiftToNewLanePolyline(actual == LCS_LEFT);

			if (Debug::Drivers) {
				DebugStream << "    Shifting to new lane." << endl;
			}

			//Check
			if (p.currLane->is_pedestrian_lane()) {
				//Flush debug output (we are debugging this error).
				if (Debug::Drivers) {
#ifndef SIMMOB_DISABLE_OUTPUT
					DebugStream << ">>>Exception: Moved to sidewalk." << endl;
					boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
					std::cout << DebugStream.str();
#endif
				}

				//TEMP OVERRIDE:
				//TODO: Fix!
				parent->setToBeRemoved();
				return;

				std::stringstream msg;
				msg << "Error: Car has moved onto sidewalk. Agent ID: " << parent->getId();
				throw std::runtime_error(msg.str().c_str());
			}

			//Set to the far edge of the other lane, minus any extra amount.
			halfLaneWidth = p.currLane->getWidth() / 2.0;
			vehicle->resetLateralMovement();
			vehicle->moveLat((halfLaneWidth - remainder) * (actual == LCS_LEFT ? -1 : 1));
		}
	} else {
		//Moving "in".

//		if(parent->getId() == 8)
//		LogOut("8,44\n");

		bool pastZero = (actual == LCS_LEFT) ? (vehicle->getLateralMovement() >= 0)
				: (vehicle->getLateralMovement() <= 0);

		if (Debug::Drivers) {
			DebugStream << "    Moving in on lane " << p.currLaneIndex << ": " << vehicle->getLateralMovement() << endl;
		}

		if (pastZero) {

			//Reset all
			vehicle->resetLateralMovement();
			vehicle->setLatVelocity(0);
			vehicle->setTurningDirection(LCS_SAME);

			if (Debug::Drivers) {
				DebugStream << "    New lane shift complete." << endl;
			}
		}

	}
}

//Retrieve the current traffic signal based on our RoadSegment's end node.
void sim_mob::Driver::saveCurrTrafficSignal() {
	const Node* node = vehicle->getCurrSegment()->getEnd();
	trafficSignal = node ? StreetDirectory::instance().signalAt(*node) : nullptr;
}

void sim_mob::Driver::setTrafficSignalParams(DriverUpdateParams& p) {


	if (!trafficSignal) {
		p.isTrafficLightStop = false;
		perceivedTrafficSignalStop.delay(p.isTrafficLightStop, p.currTimeMS);
	} else {

		Signal::TrafficColor color;
		if (vehicle->hasNextSegment(false)) {
			color = trafficSignal->getDriverLight(*p.currLane, *nextLaneInNextLink);
		} else {
			color = trafficSignal->getDriverLight(*p.currLane).forward;
		}


		switch (color) {
		case Signal::Red:
			p.isTrafficLightStop = true;
			break;

		case Signal::Amber:
		case Signal::Green:
			if (!isPedestrianOnTargetCrossing())
				p.isTrafficLightStop = false;
			else
				p.isTrafficLightStop = true;
			break;
		}
		size_t delayMS = 0;
		perceivedTrafficSignalStop.delay(p.isTrafficLightStop, p.currTimeMS);
		if(perceivedTrafficSignalStop.can_sense(p.currTimeMS))
		{
			p.perceivedTrafficSignal = perceivedTrafficSignalStop.sense(p.currTimeMS,delayMS);
			if(p.perceivedTrafficSignal)
				p.trafficSignalStopDistance = vehicle->getAllRestRoadSegmentsLength() - vehicle->getDistanceMovedInSegment() - vehicle->length / 2;
		}
	}
}

#ifndef SIMMOB_DISABLE_MPI
/**
 * Serialization
 */
void sim_mob::Driver::pack(PackageUtils& packageUtil) {
	//Part 1
	Lane::pack(packageUtil, currLane_.get());
//	packageUtil.packLane(currLane_.get());
	packageUtil.packBasicData(currLaneOffset_.get());
	packageUtil.packBasicData(currLaneLength_.get());
	packageUtil.packBasicData(isInIntersection.get());

	//part 2
	//no need to package params, params will be rebuild in the next time step
	//packageUtil.packDriverUpdateParams(params);
	Vehicle::pack(packageUtil, vehicle);
	//packageUtil.packVehicle(vehicle);
	bool hasSomething = false;
	if (intModel) {
		SimpleIntDrivingModel* simple_model = dynamic_cast<SimpleIntDrivingModel*> (intModel);
		if (simple_model) {
			hasSomething = true;
			packageUtil.packBasicData<bool>(hasSomething);
			SimpleIntDrivingModel::pack(packageUtil, simple_model);
//			packageUtil.packIntersectionDrivingModel(simple_model);
		} else {
			packageUtil.packBasicData<bool>(hasSomething);
		}
	} else {
		packageUtil.packBasicData<bool>(hasSomething);
	}

	//Part 3
	packageUtil.packFixedDelayedDPoint(perceivedVelocity);
	packageUtil.packFixedDelayedDPoint(perceivedVelocityOfFwdCar);
	packageUtil.packFixedDelayedDouble(perceivedAccelerationOfFwdCar);
	packageUtil.packFixedDelayedInt(perceivedDistToFwdCar);

	packageUtil.packPoint2D(origin.point);
	Node::pack(packageUtil, origin.node);
//	packageUtil.packNode(origin.node);
	packageUtil.packPoint2D(goal.point);
	Node::pack(packageUtil, goal.node);
//	packageUtil.packNode(goal.node);

	//packageUtil.packBasicData(firstFrameTick);
	packageUtil.packBasicData(maxLaneSpeed);

	//Part 4
	//packageUtil.packLink(desLink);
	packageUtil.packBasicData(currLinkOffset);
	packageUtil.packBasicData(targetLaneIndex);
	//packageUtil.packLink(nextLink);
	Lane::pack(packageUtil, nextLaneInNextLink);
	//packageUtil.packLane(nextLaneInNextLink);

	//Part 5
	packageUtil.packBasicData(targetSpeed);

	if(trafficSignal)
	{
		bool hasSignal = true;
		packageUtil.packBasicData<bool>(hasSignal);
		packageUtil.packPoint2D(trafficSignal->getNode().location);
	}
	else
	{
		bool hasSignal = false;
		packageUtil.packBasicData<bool>(hasSignal);
	}

	std::string value_ = DebugStream.str();
	packageUtil.packBasicData<std::string>(value_);
}

void sim_mob::Driver::unpack(UnPackageUtils& unpackageUtil) {
	//Part 1
	const Lane* one_lane = Lane::unpack(unpackageUtil);
	currLane_.force(one_lane);
	double value_lane_offset = unpackageUtil.unpackBasicData<double> ();
	currLaneOffset_.force(value_lane_offset);
	double value_lane_length = unpackageUtil.unpackBasicData<double> ();
	currLaneLength_.force(value_lane_length);
	bool value_inIntersection = unpackageUtil.unpackBasicData<bool> ();
	isInIntersection.force(value_inIntersection);

	//no need to unpackage params, params will be rebuild in the next time step
	//unpackageUtil.unpackDriverUpdateParams(params);

	//part 2
	//currTimeMS = unpackageUtil.unpackBasicData<int> ();
	vehicle = Vehicle::unpack(unpackageUtil) ;

	bool hasSomething = unpackageUtil.unpackBasicData<bool> ();
	if (hasSomething) {
		intModel = new SimpleIntDrivingModel();
		SimpleIntDrivingModel::unpack(unpackageUtil, dynamic_cast<SimpleIntDrivingModel *>(intModel)) ;
	}

	//Part 3
	perceivedVelocity = unpackageUtil.unpackFixedDelayedDPoint();
	perceivedVelocityOfFwdCar = unpackageUtil.unpackFixedDelayedDPoint();
	perceivedAccelerationOfFwdCar = unpackageUtil.unpackFixedDelayedDouble();
	perceivedDistToFwdCar = unpackageUtil.unpackFixedDelayedInt();
	std::cout << "A003" << this->getParent()->getId() << std::endl;

	origin.point = *(unpackageUtil.unpackPoint2D());
	origin.node = Node::unpack(unpackageUtil);
	goal.point = *(unpackageUtil.unpackPoint2D());
	goal.node = Node::unpack(unpackageUtil);

	//firstFrameTick = unpackageUtil.unpackBasicData<bool> ();
	maxLaneSpeed = unpackageUtil.unpackBasicData<double> ();

	//Part 4
	//desLink = unpackageUtil.unpackLink();
	currLinkOffset = unpackageUtil.unpackBasicData<double> ();
	targetLaneIndex = unpackageUtil.unpackBasicData<int> ();
	//nextLink = unpackageUtil.unpackLink();
	nextLaneInNextLink = Lane::unpack(unpackageUtil);

	std::cout << "A005" << this->getParent()->getId() << std::endl;

	//Part 5
	targetSpeed = unpackageUtil.unpackBasicData<double> ();
	bool hasSignal = unpackageUtil.unpackBasicData<bool> ();
	if(hasSignal)
	{
		Point2D* signal_location = unpackageUtil.unpackPoint2D();
		trafficSignal = sim_mob::getSignalBasedOnNode(signal_location);
	}

	std::string buffer;
	buffer = unpackageUtil.unpackBasicData<std::string> ();
	DebugStream << buffer;

	std::cout << "A006" << this->getParent()->getId() << std::endl;
}

void sim_mob::Driver::packProxy(PackageUtils& packageUtil) {
	//Part 1
	Lane::pack(packageUtil, currLane_.get());
	packageUtil.packBasicData(currLaneOffset_.get());
	packageUtil.packBasicData(currLaneLength_.get());
	packageUtil.packBasicData(isInIntersection.get());

	//packageUtil.packBasicData(currTimeMS);
	Vehicle::pack(packageUtil, vehicle);
//	packageUtil.packVehicle(vehicle);

	bool hasSomething = false;
	if(intModel)
	{
		SimpleIntDrivingModel* simple_model = dynamic_cast<SimpleIntDrivingModel*> (intModel);
		if(simple_model)
		{
			hasSomething = true;
			packageUtil.packBasicData(hasSomething);
			SimpleIntDrivingModel::pack(packageUtil, simple_model);
//			packageUtil.packIntersectionDrivingModel(simple_model);
		}
		else
		{
			packageUtil.packBasicData(hasSomething);
		}
	}
	else
	{
		packageUtil.packBasicData(hasSomething);
	}

	//Part 2
//	packageUtil.packFixedDelayedDPoint(perceivedVelocity);
//	packageUtil.packFixedDelayedDPoint(perceivedVelocityOfFwdCar);
//	packageUtil.packFixedDelayedDouble(perceivedAccelerationOfFwdCar);
//	packageUtil.packFixedDelayedInt(perceivedDistToFwdCar);
//
//	packageUtil.packPoint2D(&(origin.point));
//	packageUtil.packNode(origin.node);
//
//	packageUtil.packPoint2D(&(goal.point));
//	packageUtil.packNode(goal.node);
//
//	packageUtil.packBasicData(firstFrameTick);
//	packageUtil.packBasicData(maxLaneSpeed);
//
//	//Part 3

//	std::cout << "*****************11" << std::endl;
//	packageUtil.packLink(desLink);
	//std::cout << "*****************22" << std::endl;

	packageUtil.packBasicData(currLinkOffset);

	int lane_size = targetLaneIndex;
	packageUtil.packBasicData(lane_size);
	//std::cout << "*****************44" << std::endl;
	//packageUtil.packLink(nextLink);
	//std::cout << "*****************55" << std::endl;
	//packageUtil.packLane(nextLaneInNextLink);

//
//	//Part 4
//	packageUtil.packBasicData(targetSpeed);
	//packageUtil.packPoint2D(trafficSignal->getNode().location);
}

void sim_mob::Driver::unpackProxy(UnPackageUtils& unpackageUtil) {
	//Part 1
//	std::cout << "Driver::unpackageProxy" << std::endl;

	const Lane* one_lane = Lane::unpack(unpackageUtil);
	currLane_.force(one_lane);
//		if(this->getParent()->getId() < 1000)
//		std::cout << this->getParent()->getId() << "1-1-6-2" << std::endl;
//	std::cout << "Step 4.2.7.1:" << std::endl;

	double value_lane_offset = unpackageUtil.unpackBasicData<double> ();
	currLaneOffset_.force(value_lane_offset);
//	std::cout << "Step 4.2.7.2:" << std::endl;

//	if(this->getParent()->getId() > 1000)
//		std::cout << this->getParent()->getId() << "1-1-6-3" << std::endl;

	double value_lane_length = unpackageUtil.unpackBasicData<double> ();
	currLaneLength_.force(value_lane_length);
//	std::cout << "Step 4.2.7.3:" << std::endl;

	bool value_inIntersection = unpackageUtil.unpackBasicData<bool> ();
	isInIntersection.force(value_inIntersection);
//	std::cout << "Step 4.2.7.4:" << std::endl;

//	if(this->getParent()->getId() < 1000)
//		std::cout << this->getParent()->getId() << "1-1-6-4" << std::endl;

	//currTimeMS = unpackageUtil.unpackBasicData<int> ();
	vehicle = Vehicle::unpack(unpackageUtil);
//	std::cout << "Step 4.2.7.5:" << std::endl;

	bool hasSomething = unpackageUtil.unpackBasicData<bool> ();
	if(hasSomething)
	{
		intModel = new SimpleIntDrivingModel();

		SimpleIntDrivingModel::unpack(unpackageUtil, dynamic_cast<SimpleIntDrivingModel *>(intModel));
//				unpackageUtil.unpackIntersectionDrivingModel();
	}
//	std::cout << "Step 4.2.7.6:" << std::endl;

	//part 2 , temp
//	perceivedVelocity = unpackageUtil.unpackFixedDelayedDPoint();
//	perceivedVelocityOfFwdCar = unpackageUtil.unpackFixedDelayedDPoint();
//	perceivedAccelerationOfFwdCar = unpackageUtil.unpackFixedDelayedDouble();
//	perceivedDistToFwdCar = unpackageUtil.unpackFixedDelayedInt();
//
//	origin.point = *(unpackageUtil.unpackPoint2D());
//	origin.node = unpackageUtil.unpackNode();
//
//	goal.point = *(unpackageUtil.unpackPoint2D());
//	goal.node = unpackageUtil.unpackNode();
//
//	firstFrameTick = unpackageUtil.unpackBasicData<bool> ();
//	maxLaneSpeed = unpackageUtil.unpackBasicData<double> ();
//
//	//Part 3
//	desLink = unpackageUtil.unpackLink();

//	if(this->getParent()->getId() < 1000)
//		std::cout << this->getParent()->getId() << "1-1-6-5" << std::endl;

	currLinkOffset = unpackageUtil.unpackBasicData<double> ();

	targetLaneIndex = unpackageUtil.unpackBasicData<int> ();
	//nextLink = unpackageUtil.unpackLink();
	//nextLaneInNextLink = unpackageUtil.unpackLane();
//

//	//Part 4
//	targetSpeed = unpackageUtil.unpackBasicData<double> ();
//	Point2D* signal_location = unpackageUtil.unpackPoint2D();
	//trafficSignal = sim_mob::getSignalBasedOnNode(signal_location);
}

#endif
