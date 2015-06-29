//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Driver.cpp
 *
 *  Created on: 2011-7-5
 *      Author: wangxy & Li Zhemin
 */
 
#include "Driver.hpp"
#include "DriverFacets.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/AuraManager.hpp"
#include "entities/Person.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/UniNode.hpp"
#include "logging/Log.hpp"
#include "partitions/PartitionManager.hpp"
#include "util/DebugFlags.hpp"
#include "util/DynamicVector.hpp"
#include "util/GeomHelpers.hpp"
#include "util/ReactionTimeDistributions.hpp"
#include "entities/IntersectionManager.hpp"

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
	Role(behavior, movement, parent, roleName_, roleType_), currLane_(mtxStrat, nullptr),currTurning_(mtxStrat, nullptr), currLaneOffset_(mtxStrat, 0), currLaneLength_(mtxStrat, 0), isInIntersection_(mtxStrat, false),
	latMovement_(mtxStrat,0),fwdVelocity_(mtxStrat,0),latVelocity_(mtxStrat,0),fwdAccel_(mtxStrat,0),turningDirection_(mtxStrat,LCS_SAME),vehicle(nullptr),
	stop_event_type(mtxStrat, -1), stop_event_scheduleid(mtxStrat, -1), stop_event_lastBoardingPassengers(mtxStrat), stop_event_lastAlightingPassengers(mtxStrat), stop_event_time(mtxStrat)
	,stop_event_nodeid(mtxStrat, -1), isVehicleInLoadingQueue(true), isVehiclePositionDefined(false), moveDisOnTurning_(mtxStrat, 0),
	distToIntersection_(mtxStrat, -1), distToCurrSegmentEnd_(mtxStrat, -1), perceivedAccOfFwdCar(nullptr), perceivedDistToFwdCar(nullptr), perceivedDistToTrafficSignal(nullptr), perceivedFwdAcc(nullptr),
	perceivedFwdVel(nullptr), perceivedTrafficColor(nullptr), perceivedVelOfFwdCar(nullptr), yieldingToInIntersection(false), currDistAlongRoadSegment(0.0)
{
	getParams().driver = this;
}

void sim_mob::Driver::initReactionTime()
{
	DriverMovement* movement = dynamic_cast<DriverMovement*>(movementFacet);
	if(movement)
	{
		reactionTime = movement->getCarFollowModel()->nextPerceptionSize * 1000; // seconds to ms
	}

	perceivedFwdVel = new FixedDelayed<double>(reactionTime,true);
	perceivedFwdAcc = new FixedDelayed<double>(reactionTime,true);
	perceivedVelOfFwdCar = new FixedDelayed<double>(reactionTime,true);
	perceivedAccOfFwdCar = new FixedDelayed<double>(reactionTime,true);
	perceivedDistToFwdCar = new FixedDelayed<double>(reactionTime,true);
	perceivedDistToTrafficSignal = new FixedDelayed<double>(reactionTime,true);
	perceivedTrafficColor = new FixedDelayed<sim_mob::TrafficColor>(reactionTime,true);
}

Role* sim_mob::Driver::clone(Person* parent) const
{
	DriverBehavior* behavior = new DriverBehavior(parent);
	DriverMovement* movement = new DriverMovement(parent);
	Driver* driver = new Driver(parent, parent->getMutexStrategy(), behavior, movement);
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);
	movement->init();
	return driver;
}

void sim_mob::Driver::make_frame_tick_params(timeslice now){
	getParams().reset(now, *this);
}

// Note that Driver's destructor is only for reclaiming memory.
// If you want to remove its registered properties from the Worker (which you should do!) then
// this should occur elsewhere.
sim_mob::Driver::~Driver() {
	//Our vehicle
	safe_delete_item(vehicle);
	safe_delete_item(perceivedFwdVel);
	currResource = nullptr;
	safe_delete_item(perceivedFwdAcc);
	safe_delete_item(perceivedVelOfFwdCar);
	safe_delete_item(perceivedAccOfFwdCar);
	safe_delete_item(perceivedDistToFwdCar);
}

vector<BufferedBase*> sim_mob::Driver::getSubscriptionParams() {
	vector<BufferedBase*> res;
	res.push_back(&(currLane_));
	res.push_back(&(currTurning_));
	res.push_back(&(currLaneOffset_));
	res.push_back(&(moveDisOnTurning_));
	res.push_back(&(distToIntersection_));
	res.push_back(&(distToCurrSegmentEnd_));
	res.push_back(&(currLaneLength_));
	res.push_back(&(isInIntersection_));
	res.push_back(&(latMovement_));
	res.push_back(&(latMovement_));
	res.push_back(&(fwdVelocity_));
	res.push_back(&(latVelocity_));
	res.push_back(&(fwdAccel_));
	res.push_back(&(turningDirection_));
	res.push_back(&(stop_event_time));
	res.push_back(&(stop_event_scheduleid));
	res.push_back(&(stop_event_type));
	res.push_back(&(stop_event_nodeid));
	res.push_back(&(stop_event_lastBoardingPassengers));
	res.push_back(&(stop_event_lastAlightingPassengers));

	return res;
}

void sim_mob::Driver::onParentEvent(event::EventId eventId,
		sim_mob::event::Context ctxId,
		event::EventPublisher* sender,
		const event::EventArgs& args)
{
	if(eventId == event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH)
	{
		AMOD::AMODEventPublisher* pub = (AMOD::AMODEventPublisher*) sender;
		const AMOD::AMODRerouteEventArgs& rrArgs = MSG_CAST(AMOD::AMODRerouteEventArgs, args);
		std::cout<<"driver get reroute event <"<< rrArgs.reRoutePath.size() <<"> from <"<<pub->id<<">"<<std::endl;

		rerouteWithPath(rrArgs.reRoutePath);
	}
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

void sim_mob::Driver::handleUpdateRequest(MovementFacet* mFacet)
{
	if(this->isVehicleInLoadingQueue == false)
	{
		mFacet->updateNearbyAgent(this->getParent(),this);
	}
}

const double sim_mob::Driver::getFwdVelocityM() const
{
	double d = fwdVelocity_.get() / 100.0;
	return d;
}

double sim_mob::Driver::gapDistance(const Driver* front)
{
	double headway;
	DriverMovement* mov = dynamic_cast<DriverMovement*>(Movement());

	if (front)
	{
		/* vehicle ahead */
		DriverMovement* frontMov =
				dynamic_cast<DriverMovement*>(front->Movement());

		if (frontMov->fwdDriverMovement.isDoneWithEntireRoute())
		{
			/* vehicle ahead has already arrived at the destination */
			headway = Math::FLT_INF;
		}
		else
		{
			//if our segment is the same as that of the driver ahead
			if(mov->fwdDriverMovement.getCurrSegment() == frontMov->fwdDriverMovement.getCurrSegment())
			{
				headway = mov->fwdDriverMovement.getDisToCurrSegEnd() - frontMov->fwdDriverMovement.getDisToCurrSegEnd() - front->getVehicleLengthM();
			}
			else
			{
				/* different segment */
				headway = mov->fwdDriverMovement.getDisToCurrSegEnd() + frontMov->fwdDriverMovement.getCurrDistAlongPolylineCM() - front->getVehicleLengthM();
			}
		}
	}
	else /* no vehicle ahead. */
	{
		headway = Math::FLT_INF;
	}

	return headway;
}

bool sim_mob::Driver::isBus()
{
	return getVehicle()->getVehicleType() == VehicleBase::BUS;
}

void sim_mob::DriverUpdateParams::reset(timeslice now, const Driver& owner)
{
	UpdateParams::reset(now);

	//Set to the previous known buffered values
	if(owner.currLane_.get()) {
		currLane = owner.currLane_.get();
	}
	currLaneIndex = getLaneIndex(currLane);
	currLaneLength = owner.currLaneLength_.get();
	currLaneOffset = owner.currLaneOffset_.get();
	nextLaneIndex = currLaneIndex;

	//Current lanes to the left and right. May be null
	leftLane = nullptr;
	rightLane = nullptr;
	leftLane2 = nullptr;
	rightLane2 = nullptr;

	//Reset; these will be set before they are used; the values here represent either default
	//       values or are unimportant.
	currSpeed = 0;
	perceivedFwdVelocity = 0;
	perceivedLatVelocity = 0;
	trafficColor = sim_mob::Green;
	elapsedSeconds = ConfigManager::GetInstance().FullConfig().baseGranMS() / 1000.0;
	perceivedFwdVelocityOfFwdCar = 0;
	perceivedLatVelocityOfFwdCar = 0;
	perceivedAccelerationOfFwdCar = 0;

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
	
	density = 0;
}

void Driver::rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment*>& blacklisted)
{
	DriverMovement* mov = dynamic_cast<DriverMovement*>(Movement());
	if (mov) {
		mov->rerouteWithBlacklist(blacklisted);
	}
}

void Driver::setYieldingToInIntersection(int driverId)
{
	yieldingToInIntersection = driverId;
}

int Driver::getYieldingToInIntersection() const
{
	return yieldingToInIntersection;
}

void Driver::setCurrPosition(DPoint currPosition)
{
	currPos = currPosition;
}

const DPoint& Driver::getCurrPosition() const
{
	return currPos;
}

void Driver::resetReactionTime(double timeMS)
{
	perceivedFwdVel->set_delay(timeMS);
	perceivedFwdAcc->set_delay(timeMS);
	perceivedVelOfFwdCar->set_delay(timeMS);
	perceivedAccOfFwdCar->set_delay(timeMS);
	perceivedDistToFwdCar->set_delay(timeMS);
	perceivedDistToTrafficSignal->set_delay(timeMS);
	perceivedTrafficColor->set_delay(timeMS);
}

void Driver::rerouteWithPath(const std::vector<sim_mob::WayPoint>& path)
{
	DriverMovement* mov = dynamic_cast<DriverMovement*>(Movement());
	if (mov) {
		mov->rerouteWithPath(path);
	}
}

void Driver::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch(type)
	{
		case MSG_RESPONSE_INT_ARR_TIME:
		{
			DriverUpdateParams &params = getParams();
			const IntersectionAccess &msg = MSG_CAST(IntersectionAccess, message);
			params.response = &msg;
		}
			break;
		
		default:
			break;
	}
}