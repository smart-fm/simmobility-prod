//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
#include "logging/Log.hpp"

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
sim_mob::Driver::Driver(Person* parent, MutexStrategy mtxStrat, sim_mob::DriverBehavior* behavior, sim_mob::DriverMovement* movement, Role::type roleType_, std::string roleName_) :
	Role(behavior, movement, parent, roleName_, roleType_), currLane_(mtxStrat, nullptr), currLaneOffset_(mtxStrat, 0), currLaneLength_(mtxStrat, 0), isInIntersection(mtxStrat, false),
	latMovement(mtxStrat,0),fwdVelocity(mtxStrat,0),latVelocity(mtxStrat,0),fwdAccel(mtxStrat,0),turningDirection(mtxStrat,LCS_SAME),vehicle(nullptr),params(parent->getGenerator())
{
//	if (Debug::Drivers) {
//		DebugStream <<"Driver starting: ";
//		if (parent) { DebugStream <<parent->getId(); } else { DebugStream <<"<null>"; }
//		DebugStream <<endl;
//	}
//	trafficSignal = nullptr;
	//vehicle = nullptr;
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



//void sim_mob::Driver::frame_init(UpdateParams& p)
//{
//<<<<<<< HEAD
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
//=======
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
//		Warn() <<"ERROR: Vehicle[short] could not be created for driver; no route!" <<std::endl;
//	}
//>>>>>>> master
//}

//Main update functionality
//void sim_mob::Driver::frame_tick(UpdateParams& p)
//{
//
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
//}

//void sim_mob::Driver::frame_tick_output(const UpdateParams& p)
//{
//<<<<<<< HEAD
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
//=======
//	//Skip?
//	if (vehicle->isDone() || ConfigParams::GetInstance().using_MPI) {
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
//	const bool inLane = vehicle && (!vehicle->isInIntersection());
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
//			<<"\",\"curr-segment\":\""<<(inLane?vehicle->getCurrLane()->getRoadSegment():0x0)
//			<<"\",\"fwd-speed\":\""<<vehicle->getVelocity()
//			<<"\",\"fwd-accel\":\""<<vehicle->getAcceleration()
//			<<"\"})"<<std::endl);
//>>>>>>> master
//}

//void sim_mob::Driver::frame_tick_output_mpi(timeslice now)
//{
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
//}

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
