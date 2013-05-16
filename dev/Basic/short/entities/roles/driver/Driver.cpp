/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Driver.cpp
 *
 *  Created on: 2011-7-5
 *      Author: wangxy & Li Zhemin
 */

#include "util/ReactionTimeDistributions.hpp"
#include "Driver.hpp"
#include "DriverFacets.hpp"

#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/Person.hpp"

#include "conf/simpleconf.hpp"

#include "entities/AuraManager.hpp"
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
		const vector<RoadSegment*>& segPath = nextLink.first->getPath();
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
sim_mob::Driver::Driver(Person* parent, MutexStrategy mtxStrat, sim_mob::DriverBehavior* behavior, sim_mob::DriverMovement* movement, std::string roleName_) :
	Role(behavior, movement, parent,roleName_), currLane_(mtxStrat, nullptr), currLaneOffset_(mtxStrat, 0), currLaneLength_(mtxStrat, 0), isInIntersection(mtxStrat, false),
	latMovement(mtxStrat,0),fwdVelocity(mtxStrat,0),latVelocity(mtxStrat,0),fwdAccel(mtxStrat,0),turningDirection(mtxStrat,LCS_SAME),vehicle(nullptr),params(parent->getGenerator())
{
//	if (Debug::Drivers) {
//		DebugStream <<"Driver starting: ";
//		if (parent) { DebugStream <<parent->getId(); } else { DebugStream <<"<null>"; }
//		DebugStream <<endl;
//	}
//	trafficSignal = nullptr;
	vehicle = nullptr;
//	lastIndex = -1;
	//This is something of a quick fix; if there is no parent, then that means the
	//  reaction times haven't been initialized yet and will crash. ~Seth
	if (parent) {
		ReactionTimeDist* r1 = ConfigParams::GetInstance().reactDist1;
		ReactionTimeDist* r2 = ConfigParams::GetInstance().reactDist2;
		if (r1 && r2) {
			reacTime = r1->getReactionTime() + r2->getReactionTime();
			reacTime = 0;
		} else {
			throw std::runtime_error("Reaction time distributions have not been initialized yet.");
		}
	}

	perceivedFwdVel = new FixedDelayed<double>(reacTime,true);
	perceivedFwdAcc = new FixedDelayed<double>(reacTime,true);
	perceivedVelOfFwdCar = new FixedDelayed<double>(reacTime,true);
	perceivedAccOfFwdCar = new FixedDelayed<double>(reacTime,true);
	perceivedDistToFwdCar = new FixedDelayed<double>(reacTime,true);
	perceivedDistToTrafficSignal = new FixedDelayed<double>(reacTime,true);


	perceivedTrafficColor = new FixedDelayed<sim_mob::TrafficColor>(reacTime,true);


//	//Initialize our models. These should be swapable later.
//	lcModel = new MITSIM_LC_Model();
//	cfModel = new MITSIM_CF_Model();
//	intModel = new SimpleIntDrivingModel();
//
//	//Some one-time flags and other related defaults.
//	nextLaneInNextLink = nullptr;
//	disToFwdVehicleLastFrame = maxVisibleDis;
	// record start time
	startTime = params.now.ms()/1000.0;
	isAleadyStarted = false;
}


Role* sim_mob::Driver::clone(Person* parent) const
{
//	Role* role = 0;
//	role = new Driver(parent, parent->getMutexStrategy());
//	return role;
	DriverBehavior* behavior = new DriverBehavior(parent);
	DriverMovement* movement = new DriverMovement(parent);
	Driver* driver = new Driver(parent, parent->getMutexStrategy(), behavior, movement);
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);
	return driver;
}



void sim_mob::Driver::frame_init(UpdateParams& p)
{
//	//Save the path from orign to next activity location in allRoadSegments
//	Vehicle* newVeh = initializePath(true);
//	if (newVeh) {
//		safe_delete_item(vehicle);
//		vehicle = newVeh;
//	}
//
//	//Set some properties about the current path, such as the current polyline, etc.
//	if (vehicle && vehicle->hasPath()) {
//		setOrigin(params);
//	} else {
//		LogOut("ERROR: Vehicle[short] could not be created for driver; no route!" <<std::endl);
//	}
}

//Main update functionality
void sim_mob::Driver::frame_tick(UpdateParams& p)
{

//	//std::cout << "Driver Ticking " << p.now.frame() << std::endl;
//	// lost some params
//	DriverUpdateParams& p2 = dynamic_cast<DriverUpdateParams&>(p);
//
//	if(!vehicle)
//		throw std::runtime_error("Something wrong, Vehicle is NULL");
//	//Are we done already?
//	if (vehicle->isDone()) {
//		parent->setToBeRemoved();
//		return;
//	}
//
//	//Just a bit glitchy...
//	updateAdjacentLanes(p2);
//
//	//Update "current" time
//	perceivedFwdVel->update(params.now.ms());
//	perceivedFwdAcc->update(params.now.ms());
//	perceivedDistToFwdCar->update(params.now.ms());
//	perceivedVelOfFwdCar->update(params.now.ms());
//	perceivedAccOfFwdCar->update(params.now.ms());
//	perceivedTrafficColor->update(params.now.ms());
//	perceivedDistToTrafficSignal->update(params.now.ms());
//
//	//retrieved their current "sensed" values.
//	if (perceivedFwdVel->can_sense()) {
//		p2.perceivedFwdVelocity = perceivedFwdVel->sense();
//	}
//	else
//		p2.perceivedFwdVelocity = vehicle->getVelocity();
//
//	//General update behavior.
//	//Note: For now, most updates cannot take place unless there is a Lane and vehicle.
//	if (p2.currLane && vehicle) {
//
//		if (update_sensors(p2, p.now) && update_movement(p2, p.now) && update_post_movement(p2, p.now)) {
//
//			//Update parent data. Only works if we're not "done" for a bad reason.
//			setParentBufferedData();
//		}
//	}
//
//
//	//Update our Buffered types
//	//TODO: Update parent buffered properties, or perhaps delegate this.
//	if (!vehicle->isInIntersection()) {
//		currLane_.set(vehicle->getCurrLane());
//		currLaneOffset_.set(vehicle->getDistanceMovedInSegment());
//		currLaneLength_.set(vehicle->getCurrLinkLaneZeroLength());
//	}
//
//	isInIntersection.set(vehicle->isInIntersection());
//	latMovement.set(vehicle->getLateralMovement());
//	fwdVelocity.set(vehicle->getVelocity());
//	latVelocity.set(vehicle->getLatVelocity());
//	fwdAccel.set(vehicle->getAcceleration());
//	turningDirection.set(vehicle->getTurningDirection());
//	//Update your perceptions
//	perceivedFwdVel->delay(vehicle->getVelocity());
//	perceivedFwdAcc->delay(vehicle->getAcceleration());
//
//	//Print output for this frame.
//	disToFwdVehicleLastFrame = p2.nvFwd.distance;
}

void sim_mob::Driver::frame_tick_output(const UpdateParams& p)
{
//	//Skip?
//	if (vehicle->isDone() || ConfigParams::GetInstance().is_run_on_many_computers) {
//		return;
//	}
//
//	double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();
//
//	//Inform the GUI if interactive mode is active.
//	if (ConfigParams::GetInstance().InteractiveMode()) {
//		std::ostringstream stream;
//		stream<<"DriverSegment"
//				<<","<<p.now.frame()
//				<<","<<vehicle->getCurrSegment()
//				<<","<<vehicle->getCurrentSegmentLength()/100.0;
//		std::string s=stream.str();
//		ConfigParams::GetInstance().getCommDataMgr().sendTrafficData(s);
//	}
//
//	LogOut("(\"Driver\""
//			<<","<<p.now.frame()
//			<<","<<parent->getId()
//			<<",{"
//			<<"\"xPos\":\""<<static_cast<int>(vehicle->getX())
//			<<"\",\"yPos\":\""<<static_cast<int>(vehicle->getY())
//			<<"\",\"angle\":\""<<(360 - (baseAngle * 180 / M_PI))
//			<<"\",\"length\":\""<<static_cast<int>(vehicle->length)
//			<<"\",\"width\":\""<<static_cast<int>(vehicle->width)
//			<<"\"})"<<std::endl);
}

void sim_mob::Driver::frame_tick_output_mpi(timeslice now)
{
//	if (now.frame() < parent->getStartTime())
//		return;
//
//	if (vehicle->isDone())
//		return;
//
//	if (ConfigParams::GetInstance().OutputEnabled()) {
//		double baseAngle = vehicle->isInIntersection() ? intModel->getCurrentAngle() : vehicle->getAngle();
//		std::stringstream logout;
//
//		logout << "(\"Driver\"" << "," << now.frame() << "," << parent->getId() << ",{" << "\"xPos\":\""
//				<< static_cast<int> (vehicle->getX()) << "\",\"yPos\":\"" << static_cast<int> (vehicle->getY())
//				<< "\",\"segment\":\"" << vehicle->getCurrSegment()->getId()
//				<< "\",\"angle\":\"" << (360 - (baseAngle * 180 / M_PI)) << "\",\"length\":\""
//				<< static_cast<int> (vehicle->length) << "\",\"width\":\"" << static_cast<int> (vehicle->width);
//
//		if (this->parent->isFake) {
//			logout << "\",\"fake\":\"" << "true";
//		} else {
//			logout << "\",\"fake\":\"" << "false";
//		}
//
//		logout << "\"})" << std::endl;
//
//		LogOut(logout.str());
//	}
}

sim_mob::UpdateParams& sim_mob::Driver::make_frame_tick_params(timeslice now)
{
	params.reset(now, *this);
	return params;
}


///Note that Driver's destructor is only for reclaiming memory.
///  If you want to remove its registered properties from the Worker (which you should do!) then
///  this should occur elsewhere.
sim_mob::Driver::~Driver() {
//	//Our movement models.
//	safe_delete_item(lcModel);
//	safe_delete_item(cfModel);
//	safe_delete_item(intModel);
//
//	//Our vehicle
	safe_delete_item(vehicle);
}

vector<BufferedBase*> sim_mob::Driver::getSubscriptionParams() {
	vector<BufferedBase*> res;
	res.push_back(&(currLane_));
	res.push_back(&(currLaneOffset_));
	res.push_back(&(currLaneLength_));
	res.push_back(&(isInIntersection));
	res.push_back(&(latMovement));
	res.push_back(&(latMovement));
	res.push_back(&(fwdVelocity));
	res.push_back(&(latVelocity));
	res.push_back(&(fwdAccel));
	res.push_back(&(turningDirection));
	return res;
}

void sim_mob::DriverUpdateParams::reset(timeslice now, const Driver& owner)
{
	UpdateParams::reset(now);

	//Set to the previous known buffered values
	currLane = owner.currLane_.get();
	currLaneIndex = getLaneIndex(currLane);
	currLaneLength = owner.currLaneLength_.get();
	currLaneOffset = owner.currLaneOffset_.get();
	nextLaneIndex = currLaneIndex;

	//Current lanes to the left and right. May be null
	leftLane = nullptr;
	rightLane = nullptr;
	leftLane2 = nullptr;
	rightLane2 = nullptr;

	//Reset; these will be set before they are used; the values here represent either defaul
	//       values or are unimportant.
	currSpeed = 0;
	perceivedFwdVelocity = 0;
	perceivedLatVelocity = 0;

	trafficColor = sim_mob::Green;
	perceivedTrafficColor = sim_mob::Green;

	trafficSignalStopDistance = Driver::maxVisibleDis;
	elapsedSeconds = ConfigParams::GetInstance().baseGranMS / 1000.0;

	perceivedFwdVelocityOfFwdCar = 0;
	perceivedLatVelocityOfFwdCar = 0;
	perceivedAccelerationOfFwdCar = 0;
	perceivedDistToFwdCar = Driver::maxVisibleDis;
	perceivedDistToTrafficSignal = Driver::maxVisibleDis;

	perceivedTrafficColor  = sim_mob::Green;

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

	turningDirection = LCS_SAME;

	nvFwd.distance = Driver::maxVisibleDis;
	nvFwd = NearestVehicle();
	nvLeftFwd = NearestVehicle();
	nvRightFwd = NearestVehicle();
	nvBack = NearestVehicle();
	nvLeftBack = NearestVehicle();
	nvRightBack = NearestVehicle();
	nvLeftFwd2 = NearestVehicle();
	nvLeftBack2 = NearestVehicle();
	nvRightFwd2 = NearestVehicle();
	nvRightBack2 = NearestVehicle();
}


//bool sim_mob::Driver::update_sensors(DriverUpdateParams& params, timeslice now) {
//	//Are we done?
//	if (vehicle->isDone()) {
//		return false;
//	}
//
//	//Save the nearest agents in your lane and the surrounding lanes, stored by their
//	// position before/behind you. Save nearest fwd pedestrian too.
//
//	//Manage traffic signal behavior if we are close to the end of the link.
//	//TODO: This might be slightly inaccurate if a vehicle leaves an intersection
//	//      on a particularly short road segment. For now, though, I'm just organizing these
//	//      functions with structure in mind, and it won't affect our current network.
//	params.isApproachingToIntersection = false;
//	if (!vehicle->getNextSegment() && !vehicle->isInIntersection()) {
//		params.isApproachingToIntersection = true;
//		setTrafficSignalParams(params);
//
//	}
//
//
//	updateNearbyAgents(params);
//
//
//	return true;
//}

//bool sim_mob::Driver::update_movement(DriverUpdateParams& params, timeslice now) {
//	//If reach the goal, get back to the origin
//	if (vehicle->isDone()) {
//		//Output
//		if (Debug::Drivers && !DebugStream.str().empty()) {
//			if (ConfigParams::GetInstance().OutputEnabled()) {
//				DebugStream << ">>>Vehicle done." << endl;
//				PrintOut(DebugStream.str());
//				DebugStream.str("");
//			}
//		}
//
//		return false;
//	}
//
//
//	//Save some values which might not be available later.
//	//TODO: LastKnownPolypoint should actually be the _new_ polypoint.
//
//	const RoadSegment* prevSegment = vehicle->getCurrSegment();
//
//	params.TEMP_lastKnownPolypoint = DPoint(vehicle->getCurrPolylineVector().getEndX(),
//			vehicle->getCurrPolylineVector().getEndY());
//
//
//	//First, handle driving behavior inside an intersection.
//	if (vehicle->isInIntersection()) {
//		perceivedDistToTrafficSignal->clear();
//		perceivedTrafficColor->clear();
//		intersectionDriving(params);
//	}
//
//	//Next, handle driving on links.
//	// Note that a vehicle may leave an intersection during intersectionDriving(), so the conditional check is necessary.
//	// Note that there is no need to chain this back to intersectionDriving.
//	if (!vehicle->isInIntersection()) {
//		params.overflowIntoIntersection = linkDriving(params);
//		//Did our last move forward bring us into an intersection?
//		if (vehicle->isInIntersection()) {
//			params.justMovedIntoIntersection = true;
//			vehicle->setLatVelocity(0);
//			vehicle->setTurningDirection(LCS_SAME);
//		}
//	}
//
//
//	//Has the segment changed?
//	if ((!vehicle->isDone()) && (!vehicle->hasPath()) ) {
//		params.justChangedToNewSegment = (vehicle->getCurrSegment() != prevSegment);
//	}
//	return true;
//}

//bool sim_mob::Driver::update_post_movement(DriverUpdateParams& params, timeslice now) {
//	//Are we done?
//	if (vehicle->isDone()) {
//		return false;
//	}
//
//	//Has the segment changed?
//	if (!vehicle->isInIntersection() && params.justChangedToNewSegment) {
//		//Make pre-intersection decisions?
//		if (!vehicle->hasNextSegment(true))
//		{
//			saveCurrTrafficSignal();
////			resetPath(params);
//		}
//	}
//
//	if (!vehicle->isInIntersection() && !vehicle->hasNextSegment(true) && vehicle->hasNextSegment(false))
//		chooseNextLaneForNextLink(params);
//
//	//Have we just entered into an intersection?
//	if (vehicle->isInIntersection() && params.justMovedIntoIntersection) {
//		//Calculate a trajectory and init movement on that intersection.
//		calculateIntersectionTrajectory(params.TEMP_lastKnownPolypoint, params.overflowIntoIntersection);
//		intersectionVelocityUpdate();
//
//		//Fix: We need to perform this calculation at least once or we won't have a heading within the intersection.
//		DPoint res = intModel->continueDriving(0);
//		vehicle->setPositionInIntersection(res.x, res.y);
//	}
//
//	return true;
//}


////responsible for vehicle behaviour inside intersection
////the movement is based on absolute position
//void sim_mob::Driver::intersectionDriving(DriverUpdateParams& p) {
//	//Don't move if we have no target
//	if (!nextLaneInNextLink) {
//		return;
//	}
//
//	//First, update movement along the vector.
//	DPoint res = intModel->continueDriving(vehicle->getVelocity() * p.elapsedSeconds);
//	vehicle->setPositionInIntersection(res.x, res.y);
//
//	//Next, detect if we've just left the intersection. Otherwise, perform regular intersection driving.
//	if (intModel->isDone()) {
//		p.currLane = vehicle->moveToNextSegmentAfterIntersection();
//		if(lastIndex != -1)
//		{
//			p.currLane = vehicle->getCurrSegment()->getLane(lastIndex);
//			lastIndex = -1;
//		}
//		justLeftIntersection(p);
//	}
//}

//bool sim_mob::Driver::AvoidCrashWhenLaneChanging(DriverUpdateParams& p)
//{
//	double distanceRange =500; //currently, set 5m as the range which maybe cause two vehicles crash
//	//the subject vehicle isn't doing lane changing
//	if(vehicle->getLatVelocity()==0)
//		return false;
//	//the subject vehicle is changing to left lane
//	if(vehicle->getLatVelocity()>0 && p.nvLeftFwd2.exists() && p.nvLeftFwd2.distance < distanceRange
//			&& p.nvLeftFwd2.driver->latVelocity.get()<0)
//		return true;
//	if(vehicle->getLatVelocity()<0 && p.nvRightFwd2.exists() && p.nvRightFwd2.distance < distanceRange
//			&& p.nvRightFwd2.driver->latVelocity.get()>0)
//		return true;
//	return false;
//}

////vehicle movement on link, perform acceleration, lane changing if necessary
////the movement is based on relative position
//double sim_mob::Driver::linkDriving(DriverUpdateParams& p) {
//
//if ( (params.now.ms()/1000.0 - startTime > 10) &&  vehicle->getDistanceMovedInSegment()>2000 && isAleadyStarted == false)
//	{
//		isAleadyStarted = true;
//	}
//	p.isAlreadyStart = isAleadyStarted;
//
//	if (!vehicle->hasNextSegment(true)) // has seg in current link
//	{
//		p.dis2stop = vehicle->getAllRestRoadSegmentsLength() - vehicle->getDistanceMovedInSegment() - vehicle->length / 2 - 300;
//		if (p.nvFwd.distance < p.dis2stop)
//			p.dis2stop = p.nvFwd.distance;
//		p.dis2stop /= 100;
//	}
//	else
//	{
//		p.nextLaneIndex = std::min<int>(p.currLaneIndex, vehicle->getNextSegment()->getLanes().size() - 1);
//		if(vehicle->getNextSegment()->getLanes().at(p.nextLaneIndex)->is_pedestrian_lane())
//		{
//			p.nextLaneIndex--;
//			p.dis2stop = vehicle->getCurrPolylineLength() - vehicle->getDistanceMovedInSegment() + 1000;
//			p.dis2stop /= 100;
//		}
//		else
//			p.dis2stop = 1000;//defalut 1000m
//	}
//
//	// check current lane has connector to next link
//	if(p.dis2stop<150) // <150m need check above, ready to change lane
//	{
//////		const RoadSegment* currentSegment = vehicle->getCurrSegment();
//		const RoadSegment* nextSegment = vehicle->getNextSegment(false);
//		const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (vehicle->getNodeMovingTowards());
//		if(currEndNode)
//		{
//			// get lane connector
//			const std::set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(vehicle->getCurrSegment());
//
//			if (lcs.size()>0)
//			{
//				//
//				if(p.currLane->is_pedestrian_lane()) {
//					std::cout<<"drive on pedestrian lane"<<std::endl;
//				}
//				bool currentLaneConnectToNextLink = false;
//				int targetLaneIndex=p.currLaneIndex;
//				std::map<int,vector<int> > indexes;
//				std::set<int> noData;
//
//				for (std::set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++) {
//					if ((*it)->getLaneTo()->getRoadSegment() == nextSegment && (*it)->getLaneFrom() == p.currLane) {
//						// current lane connect to next link
//						currentLaneConnectToNextLink = true;
//						p.nextLaneIndex = p.currLaneIndex;
//						break;
//					}
//					//find target lane with same index, use this lane
//					if ((*it)->getLaneTo()->getRoadSegment() == nextSegment)
//					{
//						targetLaneIndex = getLaneIndex((*it)->getLaneFrom());
//					}
//				}
//				if( currentLaneConnectToNextLink == false ) // wow! we need change lane
//				{
//					//check target lane first
////					if(targetLaneIndex == -1) // no target lane?
////					{
////						p.nextLaneIndex = p.currLaneIndex;
//////						std::cout<<"Driver::linkDriving: can't find target lane!"<<std::endl;
////					}
////					else
//
//					p.nextLaneIndex = targetLaneIndex;
//					//NOTE: Driver already has a lcModel; we should be able to just use this. ~Seth
//					MITSIM_LC_Model* mitsim_lc_model = dynamic_cast<MITSIM_LC_Model*> (lcModel);
//					if (mitsim_lc_model) {
//						LANE_CHANGE_SIDE lcs = LCS_SAME;
////						lcs = mitsim_lc_model->makeMandatoryLaneChangingDecision(p);
//						lcs = mitsim_lc_model->makeMandatoryLaneChangingDecision(p);
//						vehicle->setTurningDirection(lcs);
//					} else {
//						throw std::runtime_error("TODO: BusDrivers currently require the MITSIM lc model.");
//					}
//				}
//			} // end of if (!lcs)
//		}
//	}
//
//
//	//Check if we should change lanes.
//	/*if (p.now.ms()/1000.0 > 41.6 && parent->getId() == 24)
//		std::cout<<"find vh"<<std::endl;*/
//	double newLatVel;
//	newLatVel = lcModel->executeLaneChanging(p, vehicle->getAllRestRoadSegmentsLength(), vehicle->length,
//			vehicle->getTurningDirection());
//
//	vehicle->setLatVelocity(newLatVel);
//	if(vehicle->getLatVelocity()>0)
//		vehicle->setTurningDirection(LCS_LEFT);
//	else if(vehicle->getLatVelocity()<0)
//		vehicle->setTurningDirection(LCS_RIGHT);
//	else
//		vehicle->setTurningDirection(LCS_SAME);
//	//when vehicle stops, don't do lane changing
////	if (vehicle->getVelocity() <= 0) {
////		vehicle->setLatVelocity(0);
////	}
//
//	p.turningDirection = vehicle->getTurningDirection();
//
//	//get nearest car, if not making lane changing, the nearest car should be the leading car in current lane.
//	//if making lane changing, adjacent car need to be taken into account.
//	NearestVehicle & nv = nearestVehicle(p);
//
//	if ( isAleadyStarted == false )
//	{
//		if(nv.distance<=0)
//		{
//			if(nv.driver->parent->getId() > parent->getId())
//			{
//				nv = NearestVehicle();
//			}
//		}
//	}
//
//
//	perceivedDataProcess(nv, p);
//
//	//Retrieve a new acceleration value.
//	double newFwdAcc = 0;
//
//	//Convert back to m/s
//	//TODO: Is this always m/s? We should rename the variable then...
//	p.currSpeed = vehicle->getVelocity() / 100;
//	//Call our model
//
//
//	newFwdAcc = cfModel->makeAcceleratingDecision(p, targetSpeed, maxLaneSpeed);
//
//	if(abs(vehicle->getTurningDirection() != LCS_SAME) && newFwdAcc>0 && vehicle->getVelocity() / 100>10)
//	{
//		newFwdAcc = 0;
//	}
//	//Update our chosen acceleration; update our position on the link.
//	vehicle->setAcceleration(newFwdAcc * 100);
////	std::cout<<"linkDriving: "<<" id: "<<parent->getId()<<" velocity: "<<vehicle->getVelocity()/100.0<<
////			" acceleration: "<<vehicle->getAcceleration()/100.0<<
////			" moveinseg: "<<vehicle->getDistanceMovedInSegment()<<std::endl;
//	return updatePositionOnLink(p);
//}

//for buffer data, maybe need more parameters to be stored
//TODO: need to discuss
//void sim_mob::Driver::setParentBufferedData() {
//	parent->xPos.set(vehicle->getX());
//	parent->yPos.set(vehicle->getY());
//
//	//TODO: Need to see how the parent agent uses its velocity vector.
//	parent->fwdVel.set(vehicle->getVelocity());
//	parent->latVel.set(vehicle->getLatVelocity());
//}

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


//bool sim_mob::Driver::isPedestrianOnTargetCrossing() const {
//	if ((!trafficSignal)||(!vehicle->getNextSegment())) {
//		return false;
//	}
//
//	//oh! we really dont neeeeeeeeed all this! r u going to repeat these two iterations for all the corresponding drivers?
////	map<Link const*, size_t> const linkMap = trafficSignal->links_map();
////	int index = -1;
////	for (map<Link const*, size_t>::const_iterator link_i = linkMap.begin(); link_i != linkMap.end(); link_i++) {
////		if (vehicle->getNextSegment() && link_i->first == vehicle->getNextSegment()->getLink()) {
////			index = (*link_i).second;
////			break;
////		}
////	}
////
////	map<Crossing const *, size_t> const crossingMap = trafficSignal->crossings_map();
////	const Crossing* crossing = nullptr;
////	for (map<Crossing const *, size_t>::const_iterator crossing_i = crossingMap.begin(); crossing_i
////			!= crossingMap.end(); crossing_i++) {
////		if (static_cast<int> (crossing_i->second) == index) {
////			crossing = crossing_i->first;
////			break;
////		}
////	}
//
//	const Crossing* crossing = nullptr;
//	const LinkAndCrossingByLink& LAC = trafficSignal->getLinkAndCrossingsByLink();
//	LinkAndCrossingByLink::iterator it = LAC.find(vehicle->getNextSegment()->getLink());
//	if(it != LAC.end())
//		const Crossing* crossing = (*it).crossing;
//
//	//Have we found a relevant crossing?
//		if (!crossing) {
//		}
//
//	//Search through the list of agents in that crossing.
//	//------------this cause error
////	vector<const Agent*> agentsInRect = GetAgentsInCrossing(crossing);
////	for (vector<const Agent*>::iterator it = agentsInRect.begin(); it != agentsInRect.end(); it++) {
////		const Person* other = dynamic_cast<const Person *> (*it);
////		if (other) {
////			const Pedestrian* pedestrian = dynamic_cast<const Pedestrian*> (other->getRole());
////			if (pedestrian && pedestrian->isOnCrossing()) {
////				return true;
////			}
////		}
////	}
//	return false;
//}

//calculate current lane length
/*void sim_mob::Driver::updateCurrLaneLength(DriverUpdateParams& p)
 {
 p.currLaneLength = vehicle->getCurrPolylineVector().getMagnitude();
 }*/

////update left and right lanes of the current lane
////if there is no left or right lane, it will be null
//void sim_mob::Driver::updateAdjacentLanes(DriverUpdateParams& p) {
//	//Need to reset, we can call this after DriverUpdateParams is initialized.
//	p.leftLane = nullptr;
//	p.rightLane = nullptr;
//	p.leftLane2 = nullptr;
//	p.rightLane2 = nullptr;
//
//	if (!p.currLane) {
//		return; //Can't do anything without a lane to reference.
//	}
//
//	if (p.currLaneIndex > 0) {
//		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex - 1);
//		if(!temp->is_pedestrian_lane())
//			p.rightLane = temp;
//	}
//	if (p.currLaneIndex > 1) {
//		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex - 2);
//		if(!temp->is_pedestrian_lane())
//			p.rightLane2 = temp;
//	}
//
//	if (p.currLaneIndex < p.currLane->getRoadSegment()->getLanes().size() - 1) {
//		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex + 1);
//		if(!temp->is_pedestrian_lane())
//			p.leftLane = temp;
//	}
//
//	if (p.currLaneIndex < p.currLane->getRoadSegment()->getLanes().size() - 2) {
//		const Lane* temp = p.currLane->getRoadSegment()->getLanes().at(p.currLaneIndex + 2);
//		if(!temp->is_pedestrian_lane())
//			p.leftLane2 = temp;
//	}
//}

////General update information for whenever a Segment may have changed.
//void sim_mob::Driver::syncCurrLaneCachedInfo(DriverUpdateParams& p) {
//	//The lane may have changed; reset the current lane index.
//	p.currLaneIndex = getLaneIndex(p.currLane);
//
//	//Update which lanes are adjacent.
//	updateAdjacentLanes(p);
//
//	//Update the length of the current road segment.
//	p.currLaneLength = vehicle->getCurrLinkLaneZeroLength();
//
//	//Finally, update target/max speed to match the new Lane's rules.
//	maxLaneSpeed = vehicle->getCurrSegment()->maxSpeed / 3.6; //slow down
//	targetSpeed = maxLaneSpeed;
//}

////currently it just chooses the first lane from the targetLane
////Note that this also sets the target lane so that we (hopefully) merge before the intersection.
//void sim_mob::Driver::chooseNextLaneForNextLink(DriverUpdateParams& p) {
//	p.nextLaneIndex = p.currLaneIndex;
//	//Retrieve the node we're on, and determine if this is in the forward direction.
//	const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (vehicle->getNodeMovingTowards());
//	const RoadSegment* nextSegment = vehicle->getNextSegment(false);
//	const RoadSegment* currentLinkNextSegment = vehicle->getNextSegment(true);
//
//	//Build up a list of target lanes.
//	nextLaneInNextLink = nullptr;
//	vector<const Lane*> targetLanes;
//	if (currEndNode && nextSegment) {
//		const set<LaneConnector*>& lcs = currEndNode->getOutgoingLanes(vehicle->getCurrSegment());
//		for (set<LaneConnector*>::const_iterator it = lcs.begin(); it != lcs.end(); it++) {
//			if ((*it)->getLaneTo()->getRoadSegment() == nextSegment && (*it)->getLaneFrom() == p.currLane) {
//				//It's a valid lane.
//				targetLanes.push_back((*it)->getLaneTo());
//
//				//find target lane with same index, use this lane
//				size_t laneIndex = getLaneIndex((*it)->getLaneTo());
//				if (laneIndex == p.currLaneIndex&&!((*it)->getLaneTo()->is_pedestrian_lane())){
//					nextLaneInNextLink = (*it)->getLaneTo();
//					targetLaneIndex = laneIndex;
//					break;
//				}
//			}
//		}
//
//		//Still haven't found a lane?
//		if (!nextLaneInNextLink) {
//			//no lane with same index, use the first lane in the vector if possible.
//			if (targetLanes.size() > 0) {
//				nextLaneInNextLink = targetLanes.at(0);
//				if(nextLaneInNextLink->is_pedestrian_lane()&&targetLanes.size()>1)
//					nextLaneInNextLink = targetLanes.at(1);
//				targetLaneIndex = getLaneIndex(nextLaneInNextLink);
//			} else if (nextSegment) { //Fallback
//				size_t fallbackIndex = std::min(p.currLaneIndex, nextSegment->getLanes().size() - 1);
//				nextLaneInNextLink = nextSegment->getLanes().at(fallbackIndex);
//				targetLaneIndex = fallbackIndex;
//			}
//		}
//
//		//We should have generated a nextLaneInNextLink here.
//		if (!nextLaneInNextLink) {
//			throw std::runtime_error("Can't find nextLaneInNextLink.");
//		}
//	}
//}

////TODO: For now, we're just using a simple trajectory model. Complex curves may be added later.
//void sim_mob::Driver::calculateIntersectionTrajectory(DPoint movingFrom, double overflow) {
//	//If we have no target link, we have no target trajectory.
//	if (!nextLaneInNextLink) {
//		LogOut("WARNING: nextLaneInNextLink has not been set; can't calculate intersection trajectory." << std::endl);
//		return;
//	}
//
//	//Get the entry point.
//	int id = getLaneIndex(vehicle->getCurrLane());
//	int startOldLane = -1;
//
//	for(vector<Lane*>::const_iterator it = vehicle->getCurrSegment()->getLanes().begin();it!=vehicle->getCurrSegment()->getLanes().end();it++)
//	{
//		if((*it)->is_pedestrian_lane() || (*it)->is_bicycle_lane())
//		{
//
//		}
//		else
//		{
//			startOldLane = getLaneIndex((*it));
//			break;
//		}
//	}
//
//
//	int total = nextLaneInNextLink->getRoadSegment()->getLanes().size()-1;
//	int offset = vehicle->getCurrSegment()->getLanes().size() -1 -id;
//	set<int> laneIDS;
//	bool first = true;
//	int StartnewLane = -1;
//	int last = 1;
//	Point2D entry = nextLaneInNextLink->getPolyline().at(0);
//
//	for(vector<Lane*>::const_iterator it = nextLaneInNextLink->getRoadSegment()->getLanes().begin();it!=nextLaneInNextLink->getRoadSegment()->getLanes().end();it++)
//	{
//		if((*it)->is_pedestrian_lane() || (*it)->is_bicycle_lane())
//		{
//
//		}
//		else
//		{
//			if(first)
//			{
//				first = false;
//				StartnewLane = getLaneIndex((*it));
//			}
//			//std::cout<<getLaneIndex((*it))<<std::endl;
//			laneIDS.insert(getLaneIndex((*it)));
//			last = getLaneIndex((*it));
//		}
//	}
//
//	if((startOldLane !=-1) && StartnewLane != -1) id = id + (StartnewLane - startOldLane);
//
//
//	if(laneIDS.find(id) != laneIDS.end())
//	{
//		entry = nextLaneInNextLink->getRoadSegment()->getLanes().at(id)->getPolyline().at(0);//  getLaneEdgePolyline(findID).at(0);
//		lastIndex = id;
//	}
//	else
//	{
//		int findID = total - offset;
//		if(findID > 0)
//		{
//			if(laneIDS.find(findID) != laneIDS.end())
//			{
//				entry = nextLaneInNextLink->getRoadSegment()->getLanes().at(findID)->getPolyline().at(0);//  getLaneEdgePolyline(findID).at(0);
//				lastIndex = findID;
//			}
//			else
//			{
//				entry = nextLaneInNextLink->getRoadSegment()->getLanes().at(last)->getPolyline().at(0);//->getLaneEdgePolyline(last).at(0);
//				lastIndex = last;
//			}
//		}
//		else
//		{
//			lastIndex = *(laneIDS.begin());
//			entry = nextLaneInNextLink->getRoadSegment()->getLanes().at(*(laneIDS.begin()))->getPolyline().at(0);//>getLaneEdgePolyline(*(laneIDS.begin())).at(0);
//		}
//
//	}
//	//Compute a movement trajectory.
//	intModel->startDriving(movingFrom, DPoint(entry.getX(), entry.getY()), overflow);
//}


//void sim_mob::Driver::initLoopSpecialString(vector<WayPoint>& path, const string& value)
//{
//	//In special cases, we may be manually specifying a loop, e.g., "loop:A:5" in the special string.
//	// In this case, "value" will contain ":A:5"; the "loop" having been parsed.
//	if (value.size()<=1) {
//		throw std::runtime_error("Bad \"loop\" special string.");
//	}
//	//Repeat this path X times.
//	vector<WayPoint> part = LoadSpecialPath(parent->originNode.node_, value[1]);
//
//	size_t ind = value.find(':', 1);
//	if (ind != string::npos && ++ind < value.length()) {
//		int amount = -1;
//		std::istringstream(value.substr(ind, string::npos)) >> amount;
//
//		for (size_t i = 0; static_cast<int> (i) < amount && amount > 1; i++) {
//			path.insert(path.end(), part.begin(), part.end());
//		}
//	} else {
//		throw std::runtime_error("Bad \"loop\" special string.");
//	}
//
//	//Just in case one of these failed.
//	if (path.empty()) {
//		path.insert(path.end(), part.begin(), part.end());
//	}
//}


namespace {
//Helper function for reading points; similar to the one in simpleconf, but it throws an
//  exception if it fails.
Point2D readPoint(const string& str) {
	//Does it match the pattern?
	size_t commaPos = str.find(',');
	if (commaPos==string::npos) {
		throw std::runtime_error("Point string badly formatted.");
	}

	//Try to parse its substrings
	int xPos, yPos;
	std::istringstream(str.substr(0, commaPos)) >> xPos;
	std::istringstream(str.substr(commaPos+1, string::npos)) >> yPos;

	return Point2D(xPos, yPos);
}
} //End anon namespace
//void sim_mob::Driver::initTripChainSpecialString(const string& value)
//{
//	//Sanity check
//	if (value.length()<=1) {
//		throw std::runtime_error("Invalid special tripchain string.");
//	}
//	Person* p = dynamic_cast<Person*>(parent);
//	if (!p) {
//		throw std::runtime_error("Parent is not of type Person");
//	}
//}



////link path should be retrieved from other class
////for now, it serves as this purpose
////Edited by Jenny (11th June)
////Try to initialize only the path from the current location to the next activity location
////Added in a parameter for the function call: next
/////Returns the new vehicle, if requested to build one.
//Vehicle* sim_mob::Driver::initializePath(bool allocateVehicle) {
//	Vehicle* res = nullptr;
//
//	//Only initialize if the next path has not been planned for yet.
//	if(!parent->getNextPathPlanned()) {
//		//Save local copies of the parent's origin/destination nodes.
//		origin.node = parent->originNode.node_;
//		origin.point = origin.node->location;
//		goal.node = parent->destNode.node_;
//		goal.point = goal.node->location;
//
//		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
//		vector<WayPoint> path;
//		Person* parentP = dynamic_cast<Person*> (parent);
//		if (!parentP || parentP->specialStr.empty()) {
//			const StreetDirectory& stdir = StreetDirectory::instance();
//			path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*origin.node), stdir.DrivingVertex(*goal.node));
//		} else {
//			//Retrieve the special string.
//			size_t cInd = parentP->specialStr.find(':');
//			string specialType = parentP->specialStr.substr(0, cInd);
//			string specialValue = parentP->specialStr.substr(cInd, std::string::npos);
//			if (specialType=="loop") {
//				initLoopSpecialString(path, specialValue);
//			} else if (specialType=="tripchain") {
//				const StreetDirectory& stdir = StreetDirectory::instance();
//				path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*origin.node), stdir.DrivingVertex(*goal.node));
//				initTripChainSpecialString(specialValue);
//			} else {
//				throw std::runtime_error("Unknown special string type.");
//			}
//		}
//
//		//For now, empty paths aren't supported.
//		if (path.empty()) {
//			throw std::runtime_error("Can't initializePath(); path is empty.");
//		}
//
//		//TODO: Start in lane 0?
//		int startlaneID = 1;
//
//		if(parentP->laneID != -1)
//		{
//			startlaneID = parentP->laneID;//need to check if lane valid
//			parentP->laneID = -1;
//		}
//
//		// Bus should be at least 1200 to be displayed on Visualizer
//		const double length = dynamic_cast<BusDriver*>(this) ? 1200 : 400;
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

void sim_mob::Driver::initializePathMed() {
//	//Only initialize if the next path has not been planned for yet.
//	if(!parent->getNextPathPlanned()){
//		//Save local copies of the parent's origin/destination nodes.
//		origin.node = parent->originNode.node_;
//		origin.point = origin.node->location;
//		goal.node = parent->destNode.node_;
//		goal.point = goal.node->location;
//
//		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
//		vector<WayPoint> path;
//		Person* parentP = dynamic_cast<Person*> (parent);
//		if (!parentP || parentP->specialStr.empty()) {
//			const StreetDirectory& stdir = StreetDirectory::instance();
//			path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*origin.node), stdir.DrivingVertex(*goal.node));
//		} else {
//			//Retrieve the special string.
//			size_t cInd = parentP->specialStr.find(':');
//			string specialType = parentP->specialStr.substr(0, cInd);
//			string specialValue = parentP->specialStr.substr(cInd, std::string::npos);
//			if (specialType=="loop") {
//				initLoopSpecialString(path, specialValue);
//			} else if (specialType=="tripchain") {
//				const StreetDirectory& stdir = StreetDirectory::instance();
//				path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*origin.node), stdir.DrivingVertex(*goal.node));
//				int x = path.size();
//				initTripChainSpecialString(specialValue);
//			} else {
//				throw std::runtime_error("Unknown special string type.");
//			}
//		}
//
//		//For now, empty paths aren't supported.
//		if (path.empty()) {
//			throw std::runtime_error("Can't initializePathMed(); path is empty.");
//		}
//	}
//
//	//to indicate that the path to next activity is already planned
//	parent->setNextPathPlanned(true);
}

////link path should be retrieved from other class
////for now, it serves as this purpose
//void sim_mob::Driver::resetPath(DriverUpdateParams& p) {
//	const Node * node = vehicle->getCurrSegment()->getEnd();
//	//Retrieve the shortest path from the current intersection node to destination and save all RoadSegments in this path.
//	const StreetDirectory& stdir = StreetDirectory::instance();
//	vector<WayPoint> path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*node), stdir.DrivingVertex(*goal.node));
//
//	//For now, empty paths aren't supported.
//	if (path.empty()) {
//		throw std::runtime_error("Can't resetPath(); path is empty.");
//	}
//
//	vector<WayPoint>::iterator it = path.begin();
//	path.insert(it, WayPoint(vehicle->getCurrSegment()));
//	vehicle->resetPath(path);
//}

//void sim_mob::Driver::setOrigin(DriverUpdateParams& p) {
//	//Set the max speed and target speed.
//	maxLaneSpeed = vehicle->getCurrSegment()->maxSpeed / 3.6;
//	targetSpeed = maxLaneSpeed;
//
//	//Set our current and target lanes.
//	p.currLane = vehicle->getCurrLane();
//	p.currLaneIndex = getLaneIndex(p.currLane);
//	targetLaneIndex = p.currLaneIndex;
//
//	//Vehicles start at rest
//	vehicle->setVelocity(0);
//	vehicle->setLatVelocity(0);
//	vehicle->setAcceleration(0);
//
//	//Calculate and save the total length of the current polyline.
//	p.currLaneLength = vehicle->getCurrLinkLaneZeroLength();
//
//	//if the first road segment is the last one in this link
//	if (!vehicle->hasNextSegment(true)) {
//		saveCurrTrafficSignal();
//	}
//	if (!vehicle->hasNextSegment(true) && vehicle->hasNextSegment(false)) {
//		//Don't do this if there is no next link.
//		chooseNextLaneForNextLink(p);
//	}
//}

////TODO
//void sim_mob::Driver::findCrossing(DriverUpdateParams& p) {
//	const Crossing* crossing = dynamic_cast<const Crossing*> (vehicle->getCurrSegment()->nextObstacle(
//			vehicle->getDistanceMovedInSegment(), true).item);
//
//	if (crossing) {
//		//TODO: Please double-check that this does what's intended.
//		Point2D interSect = LineLineIntersect(vehicle->getCurrPolylineVector(), crossing->farLine.first,
//				crossing->farLine.second);
//		DynamicVector toCrossing(vehicle->getX(), vehicle->getY(), interSect.getX(), interSect.getY());
//
//		p.crossingFwdDistance = toCrossing.getMagnitude();
//		p.isCrossingAhead = true;
//	}
//}

//double sim_mob::Driver::updatePositionOnLink(DriverUpdateParams& p) {
//	//Determine how far forward we've moved.
//	//TODO: I've disabled the acceleration component because it doesn't really make sense.
//	//      Please re-enable if you think this is expected behavior. ~Seth
//	double fwdDistance = vehicle->getVelocity() * p.elapsedSeconds + 0.5 * vehicle->getAcceleration()
//			* p.elapsedSeconds * p.elapsedSeconds;
//	if (fwdDistance < 0) {
//		fwdDistance = 0;
//	}
//
//
//	//double fwdDistance = vehicle->getVelocity()*p.elapsedSeconds;
//	double latDistance = vehicle->getLatVelocity() * p.elapsedSeconds;
//
//	//Increase the vehicle's velocity based on its acceleration.
//	vehicle->setVelocity(vehicle->getVelocity() + vehicle->getAcceleration() * p.elapsedSeconds);
//
//	//when v_lead and a_lead is 0, space is not negative, the Car Following will generate an acceleration based on free flowing model
//	//this causes problem, so i manually set acceleration and velocity to 0
//	if (vehicle->getVelocity() < 0 ||(p.space<1&&p.v_lead==0&&p.a_lead==0)) {
//		//Set to 0 forward velocity, no acceleration.
//		vehicle->setVelocity(0.0);
//		vehicle->setAcceleration(0);
//	}
//
//	//Move the vehicle forward.
//	double res = 0.0;
//	try {
//		res = vehicle->moveFwd(fwdDistance);
//		if(!vehicle->isInIntersection())
//		{
//			double d = vehicle->getDistanceMovedInSegment();
//			double c=0;
//		}
//	} catch (std::exception& ex) {
//		if (Debug::Drivers) {
//			if (ConfigParams::GetInstance().OutputEnabled()) {
//				DebugStream << ">>>Exception: " << ex.what() << endl;
//				PrintOut(DebugStream.str());
//			}
//		}
//
//		std::stringstream msg;
//		msg << "Error moving vehicle forward for Agent ID: " << parent->getId() << "," << this->vehicle->getX() << "," << this->vehicle->getY() << "\n" << ex.what();
//		throw std::runtime_error(msg.str().c_str());
//	}
//
//	//Retrieve what direction we're moving in, since it will "flip" if we cross the relative X axis.
//	LANE_CHANGE_SIDE relative = getCurrLaneSideRelativeToCenter();
//	//after forwarding, adjacent lanes might be changed
//	updateAdjacentLanes(p);
//	//there is no left lane when turning left
//	//or there is no right lane when turning right
//	if((vehicle->getTurningDirection()==LCS_LEFT && !p.leftLane)||
//			(vehicle->getTurningDirection()==LCS_RIGHT && !p.rightLane))
//	{
//		latDistance = 0;
//		vehicle->setLatVelocity(0);
//	}
//
//
//	//Lateral movement
//	if (!vehicle->isInIntersection() ) {
//		vehicle->moveLat(latDistance);
//		updatePositionDuringLaneChange(p, relative);
//	}
//
//	//Update our offset in the current lane.
//	if (!vehicle->isInIntersection()) {
//		p.currLaneOffset = vehicle->getDistanceMovedInSegment();
//	}
//	return res;
//}

//Helper function: check if a modified distance is less than the current minimum and save it.
//void sim_mob::Driver::check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh,
//		const Driver* other) {
//	//Subtract the size of the car from the distance between them
//	bool fwd=false;
//	if (distance>=0)
//		fwd = true;
//	distance = fabs(distance) - veh->length / 2 - other->getVehicleLength() / 2;
//	if ( isAleadyStarted )
//	{
//		if(fwd && distance <0)
//			distance = 0.1;
//	}
//	if (distance <= res.distance) {
//		res.driver = other;
//		res.distance = distance;
//	}
//
//}
//void sim_mob::Driver::check_and_set_min_nextlink_car_dist(NearestVehicle& res, double distance, const Vehicle* veh,
//		const Driver* other) {
//	//Subtract the size of the car from the distance between them
//	distance = fabs(distance) - other->getVehicleLength() / 2;
//	if (distance <= res.distance) {
//		res.driver = other;
//		res.distance = distance;
//	}
//}
//TODO: I have the feeling that this process of detecting nearby drivers in front of/behind you and saving them to
//      the various CFD/CBD/LFD/LBD variables can be generalized somewhat. I shortened it a little and added a
//      helper function; perhaps more cleanup can be done later? ~Seth
//void sim_mob::Driver::updateNearbyDriver(DriverUpdateParams& params, const Person* other, const Driver* other_driver) {
//	//Only update if passed a valid pointer which is not a pointer back to you, and
//	//the driver is not actually in an intersection at the moment.
//
//	/*if (params.now.ms()/1000.0 > 41.8 && parent->getId() == 25)
//			std::cout<<"find vh"<<std::endl;*/
//	if (!(other_driver && this != other_driver && !other_driver->isInIntersection.get())) {
//		return;
//	}
//
//	//Retrieve the other driver's lane, road segment, and lane offset.
//	const Lane* other_lane = other_driver->currLane_.get();
//	if (!other_lane) {
//			return;
//		}
//	const RoadSegment* otherRoadSegment = other_lane->getRoadSegment();
//
////	if (params.now.ms()/1000.0 >  93.7
////	 && parent->getId() == 402)
////	{
////			std::cout<<"find 332288222 " <<other_driver->parent->getId()<<std::endl;
////			if (otherRoadSegment->getLink() != vehicle->getCurrLink()) { //We are in the different link.
////					if (!vehicle->isInIntersection() && vehicle->getNextSegment(false) == otherRoadSegment) { //Vehicle is on the next segment,which is in next link after intersection.
////						std::cout<<"find 3322882asdfa22 " <<other_driver->parent->getId()<<std::endl;
////					}
////			}
////	}
//
//
//	if(vehicle->isInIntersection() || other_driver->vehicle->isInIntersection())
//		return;
//
////	int other_offset = other_driver->currLaneOffset_.get();
//	int other_offset = other_driver->vehicle->getDistanceMovedInSegment();
//
//	//If the vehicle is in the same Road segment
//	if (vehicle->getCurrSegment() == otherRoadSegment) {
//		//Set distance equal to the _forward_ distance between these two vehicles.
////		int distance = other_offset - params.currLaneOffset;
//		int distance = other_offset - vehicle->getDistanceMovedInSegment();
//		if (distance == 0)
//			return;
//		bool fwd = distance >= 0;
//
//		//Set different variables depending on where the car is.
//		if (other_lane == params.currLane) {//the vehicle is on the current lane
////			if (params.now.ms()/1000.0 >  123.9
////					 && parent->getId() == 404)
////			{
////					std::cout<<"find  " <<other_driver->parent->getId()<<std::endl;
////			}
//			check_and_set_min_car_dist((fwd ? params.nvFwd : params.nvBack), distance, vehicle, other_driver);
//		} else if (other_lane == params.leftLane) { //the vehicle is on the left lane
//			check_and_set_min_car_dist((fwd ? params.nvLeftFwd : params.nvLeftBack), distance, vehicle, other_driver);
//		} else if (other_lane == params.rightLane) { //The vehicle is on the right lane
//			check_and_set_min_car_dist((fwd ? params.nvRightFwd : params.nvRightBack), distance, vehicle,other_driver);
//		} else if (other_lane == params.leftLane2) { //The vehicle is on the second Left lane
//			check_and_set_min_car_dist((fwd ? params.nvLeftFwd2 : params.nvLeftBack2), distance, vehicle,other_driver);
//		} else if (other_lane == params.rightLane2) { //The vehicle is on the second right lane
//			check_and_set_min_car_dist((fwd ? params.nvRightFwd2 : params.nvRightBack2), distance, vehicle,other_driver);
//		}
//
//	} else if (otherRoadSegment->getLink() == vehicle->getCurrLink()) { //We are in the same link.
//		if (vehicle->getNextSegment() == otherRoadSegment) { //Vehicle is on the next segment.
//			//Retrieve the next node we are moving to, cast it to a UniNode.
//			const Node* nextNode = vehicle->getCurrSegment()->getEnd();
//			const UniNode* uNode = dynamic_cast<const UniNode*> (nextNode);
//			UniNode* myuode = const_cast<sim_mob::UniNode*> (uNode);
////			if (params.now.ms()/1000.0 >  123.9
////					 && parent->getId() == 404)
////			{
////					std::cout<<"find 65298 " <<parent->getId()<<std::endl;
////			}
//			//Initialize some lane pointers
//			const Lane* nextLane = nullptr;
//			const Lane* nextLeftLane = nullptr;
//			const Lane* nextRightLane = nullptr;
//			const Lane* nextLeftLane2 = nullptr;
//			const Lane* nextRightLane2 = nullptr;
////			if (uNode) {
////				nextLane = uNode->getOutgoingLane(*params.currLane);
////			}
//
//			if (uNode) {
//				nextLane = uNode->getForwardDrivingLane(*params.currLane);
//			}
//
////			//
////			const sim_mob::Lane * currlan = params.currLane;
////			std::vector<Point2D> currployline = currlan->polyline_;
////			int s = currployline.size() - 1;
////			Point2D currlastp = currployline.back();
////			//
////			std::vector<sim_mob::Lane*> outlanes;
////			if (uNode) {
////				sim_mob::Lane *l = const_cast<sim_mob::Lane*> (params.currLane);
////				outlanes = myuode->getOutgoingLanes(*l);
////			}
////			if (outlanes.size() == 0)
////				std::cout<<"error no out lane"<<std::endl;
////			double dis=100000000;
////			for (std::vector< sim_mob::Lane*>::iterator it=outlanes.begin();it!=outlanes.end();++it)
////			{
////				Lane * l = *it;
////				std::vector<Point2D> pl = l->polyline_;
////				Point2D plzero = currployline[0];
////				DynamicVector zeroPoly(currlastp.getX(), currlastp.getY(), plzero.getX(), plzero.getY());
////				double d = zeroPoly.getMagnitude();
////				if(d<dis)
////				{
////					nextLane = l;
////				}
////			}
//
//			//Make sure next lane exists and is in the next road segment, although it should be true
//			if (nextLane && nextLane->getRoadSegment() == otherRoadSegment) {
//				//Assign next left/right lane based on lane ID.
//				size_t nextLaneIndex = getLaneIndex(nextLane);
//				if (nextLaneIndex > 0) {
//					nextLeftLane = otherRoadSegment->getLanes().at(nextLaneIndex - 1);
//				}
//				if (nextLaneIndex < otherRoadSegment->getLanes().size() - 1) {
//					nextRightLane = otherRoadSegment->getLanes().at(nextLaneIndex + 1);
//				}
//				if (nextLaneIndex > 1)
//				{
//					nextLeftLane2 = otherRoadSegment->getLanes().at(nextLaneIndex - 2);
//				}
//				if (nextLaneIndex < otherRoadSegment->getLanes().size() - 2) {
//				    nextRightLane2 = otherRoadSegment->getLanes().at(nextLaneIndex + 2);
//				}
//			}
//
//			//Modified distance.
//			int distance = other_offset + params.currLaneLength - params.currLaneOffset;
//
//			//Set different variables depending on where the car is.
//			if (other_lane == nextLane) { //The vehicle is on the current lane
////				if (params.now.ms()/1000.0 >  123.9
////						 && parent->getId() == 404)
////				{
////						std::cout<<"find 65298 " <<other_driver->parent->getId()<<std::endl;
////				}
//				check_and_set_min_car_dist(params.nvFwd, distance, vehicle, other_driver);
//			} else if (other_lane == nextLeftLane) { //the vehicle is on the left lane
//				check_and_set_min_car_dist(params.nvLeftFwd, distance, vehicle, other_driver);
//			} else if (other_lane == nextRightLane) { //the vehicle is in front
//				check_and_set_min_car_dist(params.nvRightFwd, distance, vehicle, other_driver);
//			} else if (other_lane == nextLeftLane2) { //The vehicle is on the second Left lane
//				check_and_set_min_car_dist(params.nvLeftFwd2, distance, vehicle,other_driver);
//			} else if (other_lane == nextRightLane2) { //The vehicle is on the second right lane
//				check_and_set_min_car_dist(params.nvRightFwd2, distance, vehicle,other_driver);
//			}
//		} else if (vehicle->getPrevSegment() == otherRoadSegment) { //Vehicle is on the previous segment.
//			//Retrieve the previous node as a UniNode.
//			const Node* prevNode = vehicle->getNodeMovingFrom();
//			const UniNode* uNode = dynamic_cast<const UniNode*> (prevNode);
//
//			//Set some lane pointers.
//			const Lane* preLane = nullptr;
//			const Lane* preLeftLane = nullptr;
//			const Lane* preRightLane = nullptr;
//			const Lane* preLeftLane2 = nullptr;
//			const Lane* preRightLane2 = nullptr;
//
//			//Find the node which leads to this one from the UniNode. (Requires some searching; should probably
//			//   migrate this to the UniNode class later).
//			const vector<Lane*>& lanes = otherRoadSegment->getLanes();
//			if (uNode) {
//				for (vector<Lane*>::const_iterator it = lanes.begin(); it != lanes.end() && !preLane; it++) {
//					if (uNode->getOutgoingLane(**it) == params.currLane) {
//						preLane = *it;
//					}
//				}
//			}
//
//			//Make sure next lane is in the next road segment, although it should be true
//			if (preLane) {
//				//Save the new left/right lanes
//				size_t preLaneIndex = getLaneIndex(preLane);
//				if (preLaneIndex > 0) {
//					preLeftLane = otherRoadSegment->getLanes().at(preLaneIndex - 1);
//				}
//				if (preLaneIndex < otherRoadSegment->getLanes().size() - 1) {
//					preRightLane = otherRoadSegment->getLanes().at(preLaneIndex + 1);
//				}
//			}
//
//			//Modified distance.
//			int distance = other_driver->currLaneLength_.get() - other_offset + params.currLaneOffset;
//
//			//Set different variables depending on where the car is.
//			if (other_lane == preLane) { //The vehicle is on the current lane
//				check_and_set_min_car_dist(params.nvBack, distance, vehicle, other_driver);
//			} else if (other_lane == preLeftLane) { //the vehicle is on the left lane
//				check_and_set_min_car_dist(params.nvLeftBack, distance, vehicle, other_driver);
//			} else if (other_lane == preRightLane) { //the vehicle is on the right lane
//				check_and_set_min_car_dist(params.nvRightBack, distance, vehicle, other_driver);
//			} else if (other_lane == preLeftLane2) { //The vehicle is on the second Left lane
//				check_and_set_min_car_dist(params.nvLeftBack2, distance, vehicle,other_driver);
//			} else if (other_lane == preRightLane2) { //The vehicle is on the second right lane
//				check_and_set_min_car_dist(params.nvRightBack2, distance, vehicle,other_driver);
//			}
//		}
//	}
//
//	if (otherRoadSegment->getLink() != vehicle->getCurrLink()) { //We are in the different link.
//		if (!vehicle->isInIntersection() && vehicle->getNextSegment(false) == otherRoadSegment) { //Vehicle is on the next segment,which is in next link after intersection.
//			// 1. host vh's target lane is == other_driver's lane
//			//
////			if (params.now.ms()/1000.0 >  93.7
////					 && parent->getId() == 402)
////			{
////					std::cout<<"find 332288 " <<other_driver->parent->getId()<<std::endl;
////			}
//			size_t targetLaneIndex = params.nextLaneIndex;
//			size_t otherVhLaneIndex = getLaneIndex(other_lane);
//			if (targetLaneIndex == otherVhLaneIndex)
//			{
//
//				if (params.now.ms()/1000.0 == 92.8)
//				{
//					int a=1;
//				}
//				if (params.nvFwd.driver==NULL)
//				{
////					std::cout<<"find this " <<other_driver->parent->getId()<<std::endl;
//					// 2. other_driver's distance move in the segment, it is also the distance vh to intersection
//					double currSL = vehicle->getCurrentSegmentLength();
//					double disMIS = vehicle->getDistanceMovedInSegment();
//					double otherdis = other_driver->vehicle->getDistanceMovedInSegment();
//					double distance =  currSL- disMIS + otherdis;
//					// 3. compare the distance and set params.nvFwdNextLink
//					check_and_set_min_nextlink_car_dist(params.nvFwdNextLink, distance, vehicle, other_driver);
//				}
//			}
//		}
//	} // end of in different link
//
//}

//void sim_mob::Driver::updateNearbyPedestrian(DriverUpdateParams& params, const Person* other, const Pedestrian* pedestrian) {
//	//Only update if passed a valid pointer and this is on a crossing.
//
//	if (!(pedestrian && pedestrian->isOnCrossing())) {
//		return;
//	}
//
//	//TODO: We are using a vector to check the angle to the Pedestrian. There are other ways of doing this which may be more accurate.
//	const std::vector<sim_mob::Point2D>& polyLine = vehicle->getCurrSegment()->getLanes().front()->getPolyline();
//	DynamicVector otherVect(polyLine.front().getX(), polyLine.front().getY(), other->xPos.get(), other->yPos.get());
//
//	//Calculate the distance between these two vehicles and the distance between the angle of the
//	// car's forward movement and the pedestrian.
//	//NOTE: I am changing this slightly, since cars were stopping for pedestrians on the opposite side of
//	//      the road for no reason (traffic light was green). ~Seth
//	//double distance = otherVect.getMagnitude();
//	double angleDiff = 0.0;
//	{
//		//Retrieve
//		DynamicVector fwdVector(vehicle->getCurrPolylineVector());
//		fwdVector.scaleVectTo(100);
//
//		//Calculate the difference
//		//NOTE: I may be over-complicating this... we can probably use the dot product but that can be done later. ~Seth
//		double angle1 = atan2(fwdVector.getEndY() - fwdVector.getY(), fwdVector.getEndX() - fwdVector.getX());
//		double angle2 = atan2(otherVect.getEndY() - otherVect.getY(), otherVect.getEndX() - otherVect.getX());
//		double diff = fabs(angle1 - angle2);
//		angleDiff = std::min(diff, fabs(diff - 2 * M_PI));
//	}
//
//	//If the pedestrian is not behind us, then set our flag to true and update the minimum pedestrian distance.
//	if (angleDiff < 0.5236) { //30 degrees +/-
//		params.npedFwd.distance = std::min(params.npedFwd.distance, otherVect.getMagnitude() - vehicle->length / 2
//				- 300);
//	}
//}

//void sim_mob::Driver::updateNearbyAgents(DriverUpdateParams& params) {
//	//Retrieve a list of nearby agents
//
//	double dis = 10000.0;
//	vector<const Agent*> nearby_agents = AuraManager::instance().nearbyAgents(
//			Point2D(vehicle->getX(), vehicle->getY()), *params.currLane, dis, distanceBehind);
//
//	//Update each nearby Pedestrian/Driver
//
//	//
//	params.nvFwdNextLink.driver=NULL;
//	params.nvFwdNextLink.distance = 50000;
//	params.nvFwd.driver=NULL;
//	params.nvFwd.distance = 50000;
//
////	if (params.now.ms()/1000.0 >  93.7
////			 && parent->getId() == 402)
////		{
////			std::cout<<"asdf"<<std::endl;
////		}
//
//	for (vector<const Agent*>::iterator it = nearby_agents.begin(); it != nearby_agents.end(); it++) {
//		//Perform no action on non-Persons
//		const Person* other = dynamic_cast<const Person *> (*it);
//		if (!other) {
//			continue;
//		}
//
//		//Perform a different action depending on whether or not this is a Pedestrian/Driver/etc.
//		updateNearbyDriver(params, other, dynamic_cast<const Driver*> (other->getRole()));
//		updateNearbyPedestrian(params, other, dynamic_cast<const Pedestrian*> (other->getRole()));
//	}
//}

//void sim_mob::Driver::perceivedDataProcess(NearestVehicle & nv, DriverUpdateParams& params)
//{
//	//Update your perceptions for leading vehicle and gap
//	if (nv.exists()) {
//
//		if (reacTime == 0)
//		{
//			params.perceivedFwdVelocityOfFwdCar = nv.driver?nv.driver->fwdVelocity.get():0;
//			params.perceivedLatVelocityOfFwdCar = nv.driver?nv.driver->latVelocity.get():0;
//			params.perceivedAccelerationOfFwdCar = nv.driver?nv.driver->fwdAccel.get():0;
//			params.perceivedDistToFwdCar = nv.distance;
//
//			return;
//		}
//		//Change perception delay
//		perceivedDistToFwdCar->set_delay(reacTime);
//		perceivedVelOfFwdCar->set_delay(reacTime);
//		perceivedAccOfFwdCar->set_delay(reacTime);
//
//		//Now sense.
//		if (perceivedVelOfFwdCar->can_sense()
//				&&perceivedAccOfFwdCar->can_sense()
//				&&perceivedDistToFwdCar->can_sense()) {
//			params.perceivedFwdVelocityOfFwdCar = perceivedVelOfFwdCar->sense();
//			params.perceivedAccelerationOfFwdCar = perceivedAccOfFwdCar->sense();
//			params.perceivedDistToFwdCar = perceivedDistToFwdCar->sense();
////			std::cout<<"perceivedDataProcess: perceivedFwdVelocityOfFwdCar: vel,acc,dis:"<<params.perceivedFwdVelocityOfFwdCar
////					<<" "<<params.perceivedAccelerationOfFwdCar<<" "<<
////					params.perceivedDistToFwdCar<<std::endl;
//		}
//		else
//		{
//			params.perceivedFwdVelocityOfFwdCar = nv.driver?nv.driver->fwdVelocity.get():0;
//			params.perceivedLatVelocityOfFwdCar = nv.driver?nv.driver->latVelocity.get():0;
//			params.perceivedAccelerationOfFwdCar = nv.driver?nv.driver->fwdAccel.get():0;
//			params.perceivedDistToFwdCar = nv.distance;
//		}
//		perceivedDistToFwdCar->delay(nv.distance);
//		perceivedVelOfFwdCar->delay(nv.driver->fwdVelocity.get());
//		perceivedAccOfFwdCar->delay(nv.driver->fwdAccel.get());
//	}
//
//
//}

//NearestVehicle & sim_mob::Driver::nearestVehicle(DriverUpdateParams& p)
//{
//	double leftDis = 5000;
//	double rightDis = 5000;
//	double currentDis = 5000;
//	p.isBeforIntersecton = false;
//	if(p.nvLeftFwd.exists())
//	  leftDis = p.nvLeftFwd.distance;
//	if(p.nvRightFwd.exists())
//	  rightDis = p.nvRightFwd.distance;
//	if(p.nvFwd.exists())
//	{
//	  currentDis = p.nvFwd.distance;
//	}
//	else if(p.nvFwdNextLink.exists() && p.turningDirection == LCS_SAME)
//	{
//		currentDis = p.nvFwdNextLink.distance;
//		p.isBeforIntersecton = true;
////		if (currentDis<200 && params.now.ms()/1000.0 > 100.0 )
////		{
////			std::cout<<"find one"<<std::endl;
////		}
//		/*if (params.now.ms()/1000.0 > 41.8 && parent->getId() == 25)
//					std::cout<<"find vh"<<std::endl;*/
//		return p.nvFwdNextLink;
//	}
//	if(leftDis<currentDis)
//	{
//		//the vehicle in the left lane is turning to right
//		//or subject vehicle is turning to left
//		if(p.nvLeftFwd.driver->turningDirection.get()==LCS_RIGHT &&
//				vehicle->getTurningDirection()==LCS_LEFT && p.nvLeftFwd.driver->getVehicle()->getVelocity()>500)
//		{
////			std::cout<<"nearestVehicle: left forward"<<std::endl;
//			return p.nvLeftFwd;
//		}
//	}
//	else if(rightDis<currentDis)
//	{
//		if(p.nvRightFwd.driver->turningDirection.get()==LCS_LEFT &&
//				vehicle->getTurningDirection()==LCS_RIGHT && p.nvRightFwd.driver->getVehicle()->getVelocity()>500)
//		{
////			std::cout<<"nearestVehicle: right forward: rightDis,currentDis: "<<rightDis<<" "<<currentDis<<std::endl;
//			return p.nvRightFwd;
//		}
//	}
////	if (p.nvFwd.exists())
////		std::cout<<"nearestVehicle: forward"<<std::endl;
//	return p.nvFwd;
//}

//void sim_mob::Driver::intersectionVelocityUpdate() {
//	double inter_speed = 1000;//10m/s
//	vehicle->setAcceleration(0);
//
//	//Set velocity for intersection movement.
//	vehicle->setVelocity(inter_speed);
//}
//
//void sim_mob::Driver::justLeftIntersection(DriverUpdateParams& p) {
//	//p.currLane = nextLaneInNextLink;
//	p.currLaneIndex = getLaneIndex(p.currLane);
//	vehicle->moveToNewLanePolyline(p.currLaneIndex);
//	syncCurrLaneCachedInfo(p);
//	p.currLaneOffset = vehicle->getDistanceMovedInSegment();
//	targetLaneIndex = p.currLaneIndex;
//
//	//Reset lateral movement/velocity to zero.
//	vehicle->setLatVelocity(0);
//	vehicle->resetLateralMovement();
//}

//LANE_CHANGE_SIDE sim_mob::Driver::getCurrLaneChangeDirection() const {
//	if (vehicle->getLatVelocity() > 0) {
//		return LCS_LEFT;
//	} else if (vehicle->getLatVelocity() < 0) {
//		return LCS_RIGHT;
//	}
//	return LCS_SAME;
//}
//
//LANE_CHANGE_SIDE sim_mob::Driver::getCurrLaneSideRelativeToCenter() const {
//	if (vehicle->getLateralMovement() > 0) {
//		return LCS_LEFT;
//	} else if (vehicle->getLateralMovement() < 0) {
//		return LCS_RIGHT;
//	}
//	return LCS_SAME;
//}

////TODO: I think all lane changing occurs after 150m. Double-check please. ~Seth
//void sim_mob::Driver::updatePositionDuringLaneChange(DriverUpdateParams& p, LANE_CHANGE_SIDE relative) {
//
//	double halfLaneWidth = p.currLane->getWidth() / 2.0;
//
//	//The direction we are attempting to change lanes in
//	LANE_CHANGE_SIDE actual = vehicle->getTurningDirection();
//	//LANE_CHANGE_SIDE relative = getCurrLaneSideRelativeToCenter();
//	if (actual == LCS_SAME && relative == LCS_SAME) {
//		if (Debug::Drivers) {
//			DebugStream << "  Lane change: " << "ERROR: Called with \"same\"" << endl;
//		}
//		return; //Not actually changing lanes.
//	}
//	if (relative == LCS_SAME) {
//		relative = actual; //Unlikely to occur, but we can still work off this.
//	}
//	if (actual == relative) { //We haven't merged halfway yet; check there's actually a lane for us to merge into.
//		if ((actual == LCS_LEFT && !p.leftLane) || (actual == LCS_RIGHT && !p.rightLane)) {
//			std::stringstream msg;
//			msg <<"Agent (" <<parent->getId() <<") is attempting to merge into a lane that doesn't exist.";
//			throw std::runtime_error(msg.str().c_str());
//			//return; //Error condition
//		}
//	}
//
//	if (Debug::Drivers) {
//		DebugStream << "  Lane change: " << PrintLCS(actual) << " (" << PrintLCS(relative) << ")" << endl;
//	}
//
//	//Basically, we move "halfway" into the next lane, and then move "halfway" back to its midpoint.
//	if (actual == relative) {
//		//Moving "out".
//		double remainder = fabs(vehicle->getLateralMovement()) - halfLaneWidth;
//
//		if (Debug::Drivers) {
//			DebugStream << "    Moving out on Lane " << p.currLaneIndex << ": " << remainder << endl;
//		}
//
//		if (remainder >= 0) {
//			//Update Lanes, polylines, RoadSegments, etc.
//			p.currLane = (actual == LCS_LEFT ? p.leftLane : p.rightLane);
//			syncCurrLaneCachedInfo(p);
//
//			vehicle->shiftToNewLanePolyline(actual == LCS_LEFT);
//
//			if (Debug::Drivers) {
//				DebugStream << "    Shifting to new lane." << endl;
//			}
//
//			//Check
//			if (p.currLane->is_pedestrian_lane()) {
//				//Flush debug output (we are debugging this error).
//				if (Debug::Drivers) {
//					if (ConfigParams::GetInstance().OutputEnabled()) {
//						DebugStream << ">>>Exception: Moved to sidewalk." << endl;
//						PrintOut(DebugStream.str());
//					}
//				}
//
//				//TEMP OVERRIDE:
//				//TODO: Fix!
//				parent->setToBeRemoved();
//				return;
//
//				std::stringstream msg;
//				msg << "Error: Car has moved onto sidewalk. Agent ID: " << parent->getId();
//				throw std::runtime_error(msg.str().c_str());
//			}
//
//			//Set to the far edge of the other lane, minus any extra amount.
//			halfLaneWidth = p.currLane->getWidth() / 2.0;
//			vehicle->resetLateralMovement();
//			vehicle->moveLat((halfLaneWidth - remainder) * (actual == LCS_LEFT ? -1 : 1));
//		}
//	} else {
//		//Moving "in".
//		if (fabs(vehicle->getLateralMovement()) < 30)
//		{
//			vehicle->resetLateralMovement();
//			return;
//		}
//		if (relative == LCS_LEFT)
//			vehicle->moveLat(-15);
//		else
//			vehicle->moveLat(15);
////		bool pastZero = (relative == LCS_LEFT) ? (vehicle->getLateralMovement() >= 0)
////				: (vehicle->getLateralMovement() <= 0);
////
////		if (Debug::Drivers) {
////			DebugStream << "    Moving in on lane " << p.currLaneIndex << ": " << vehicle->getLateralMovement() << endl;
////		}
////
////		if (pastZero) {
////
////			//Reset all
////			vehicle->resetLateralMovement();
////			vehicle->setLatVelocity(0);
////			vehicle->setTurningDirection(LCS_SAME);
////
////			if (Debug::Drivers) {
////				DebugStream << "    New lane shift complete." << endl;
////			}
////		}
//
//	}
//}

////Retrieve the current traffic signal based on our RoadSegment's end node.
//void sim_mob::Driver::saveCurrTrafficSignal() {
////	const Node* node = vehicle->getCurrSegment()->getEnd();
//	const Node* node;
//	if(vehicle->isMovingForwardsInLink())
//		node = vehicle->getCurrLink()->getEnd();
//	else
//		node = vehicle->getCurrLink()->getStart();
//	trafficSignal = node ? StreetDirectory::instance().signalAt(*node) : nullptr;
//}
//
//void sim_mob::Driver::setTrafficSignalParams(DriverUpdateParams& p) {
//
//
//	if (!trafficSignal) {
//			p.trafficColor = sim_mob::Green;
//
//		perceivedTrafficColor->delay(p.trafficColor);
//	} else {
//		sim_mob::TrafficColor color;
//
//		if (vehicle->hasNextSegment(false)) {
//
////			std::cout << "In Driver::setTrafficSignalParams, frame number " << p.frameNumber <<
////					"  Getting the driver light from lane " << p.currLane  <<
////					"(" << p.currLane->getRoadSegment()->getLink()->roadName << ")" <<
////					" To "<< nextLaneInNextLink  <<
////					"(" << nextLaneInNextLink->getRoadSegment()->getLink()->roadName << ")" << std::endl;
//
//
//			color = trafficSignal->getDriverLight(*p.currLane, *nextLaneInNextLink);
//
//
////			std::cout << "The driver light is " << color << std::endl;
//		} else {
//			/*vahid:
//			 * Basically,there is no notion of left, right forward any more.
//			 * (I said "Basically" coz I can think of at least one "if" :left turn in singapore, right turn in US...)
//			 * so it is omitted by  If you insist on having this type of function, I can give you a vector/container
//			 * of a map between lane/link and their corresponding current color with respect to the currLane
//			 * todo:think of something for this else clause! you are going continue with No color!S
//			 */
////			color = trafficSignal->getDriverLight(*p.currLane).forward;
//		}
//		switch (color) {
//		case sim_mob::Red:
//
////			std::cout<< "Driver is getting Red light \n";
//			p.trafficColor = color;
//			break;
//		case sim_mob::Amber:
//		case sim_mob::Green:
//
////			std::cout<< "Driver is getting Green or Amber light \n";
//			if (!isPedestrianOnTargetCrossing())
//				p.trafficColor = color;
//			else
//				p.trafficColor = sim_mob::Red;
//				break;
//		}
//
//		perceivedTrafficColor->set_delay(reacTime);
//		if(perceivedTrafficColor->can_sense())
//		{
//			p.perceivedTrafficColor = perceivedTrafficColor->sense();
//		}
//		else
//			p.perceivedTrafficColor = color;
//
//		perceivedTrafficColor->delay(p.trafficColor);
//
//
//		p.trafficSignalStopDistance = vehicle->getAllRestRoadSegmentsLength() - vehicle->getDistanceMovedInSegment() - vehicle->length / 2;
//		perceivedDistToTrafficSignal->set_delay(reacTime);
//		if(perceivedDistToTrafficSignal->can_sense())
//		{
//			p.perceivedDistToTrafficSignal = perceivedDistToTrafficSignal->sense();
//		}
//		else
//			p.perceivedDistToTrafficSignal = p.trafficSignalStopDistance;
//		perceivedDistToTrafficSignal->delay(p.trafficSignalStopDistance);
//	}
//}
