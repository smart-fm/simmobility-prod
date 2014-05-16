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

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
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

// millisecs conversion unit from seconds
const double MILLISECS_CONVERT_UNIT = 1000.0;

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


} //End anon namespace


//Initialize
sim_mob::Driver::Driver(Person* parent, MutexStrategy mtxStrat, sim_mob::DriverBehavior* behavior, sim_mob::DriverMovement* movement, Role::type roleType_, std::string roleName_) :
	Role(behavior, movement, parent, roleName_, roleType_), currLane_(mtxStrat, nullptr), currLaneOffset_(mtxStrat, 0), currLaneLength_(mtxStrat, 0), isInIntersection(mtxStrat, false),
	latMovement(mtxStrat,0),fwdVelocity(mtxStrat,0),latVelocity(mtxStrat,0),fwdAccel(mtxStrat,0),turningDirection(mtxStrat,LCS_SAME),vehicle(nullptr),/*params(parent->getGenerator()),*/
	stop_event_type(mtxStrat, -1), stop_event_scheduleid(mtxStrat, -1), stop_event_lastBoardingPassengers(mtxStrat), stop_event_lastAlightingPassengers(mtxStrat), stop_event_time(mtxStrat)
	,stop_event_nodeid(mtxStrat, -1)
{
	//This is something of a quick fix; if there is no parent, then that means the
	//  reaction times haven't been initialized yet and will crash. ~Seth
	if (parent) {
		ReactionTimeDist* r1 = ConfigManager::GetInstance().FullConfig().reactDist1;
		ReactionTimeDist* r2 = ConfigManager::GetInstance().FullConfig().reactDist2;
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

	// record start time
	startTime = getParams().now.ms()/MILLISECS_CONVERT_UNIT;
	isAleadyStarted = false;
	currDistAlongRoadSegment = 0;
}


Role* sim_mob::Driver::clone(Person* parent) const
{
	DriverBehavior* behavior = new DriverBehavior(parent);
	DriverMovement* movement = new DriverMovement(parent);
	Driver* driver = new Driver(parent, parent->getMutexStrategy(), behavior, movement);
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);
	return driver;
}





void sim_mob::Driver::make_frame_tick_params(timeslice now){
	getParams().reset(now, *this);
}


///Note that Driver's destructor is only for reclaiming memory.
///  If you want to remove its registered properties from the Worker (which you should do!) then
///  this should occur elsewhere.
sim_mob::Driver::~Driver() {
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
	res.push_back(&(stop_event_time));
	res.push_back(&(stop_event_scheduleid));
	res.push_back(&(stop_event_type));
	res.push_back(&(stop_event_nodeid));
	res.push_back(&(stop_event_lastBoardingPassengers));
	res.push_back(&(stop_event_lastAlightingPassengers));

	return res;
}

std::vector<sim_mob::BufferedBase*> sim_mob::Driver::getDriverInternalParams()
{
	vector<BufferedBase*> res;
	res.push_back(&(stop_event_time));
	res.push_back(&(stop_event_type));
	res.push_back(&(stop_event_scheduleid));
	res.push_back(&(stop_event_nodeid));
	res.push_back(&(stop_event_lastBoardingPassengers));
	res.push_back(&(stop_event_lastAlightingPassengers));

	return res;
}

void sim_mob::Driver::handleUpdateRequest(MovementFacet* mFacet){
	mFacet->updateNearbyAgent(this->getParent(),this);
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
	elapsedSeconds = ConfigManager::GetInstance().FullConfig().baseGranMS() / 1000.0;

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

void Driver::rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment*>& blacklisted)
{
	DriverMovement* mov = dynamic_cast<DriverMovement*>(Movement());
	if (mov) {
		mov->rerouteWithBlacklist(blacklisted);
	}
}

void Driver::setCurrPosition(DPoint currPosition)
{
	currPos = currPosition;
}

const DPoint& Driver::getCurrPosition() const
{
	return currPos;
}
