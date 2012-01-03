/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "GenConfig.h"
#include "entities/UpdateParams.hpp"
#include "util/DynamicVector.hpp"


namespace sim_mob
{

//Forward declarations
class Lane;
class Driver;


struct LaneSide {
	bool left;
	bool right;
	bool both() const { return left && right; }
	bool leftOnly() const { return left && !right; }
	bool rightOnly() const { return right && !left; }
};

enum LANE_CHANGE_MODE {	//as a mask
	DLC = 0,
	MLC = 2
};

enum LANE_CHANGE_SIDE {
	LCS_LEFT = -1,
	LCS_SAME = 0,
	LCS_RIGHT = 1
};


//Struct for holding data about the "nearest" vehicle.
struct NearestVehicle {
	NearestVehicle() : driver(nullptr), distance(5000) {}

	const Driver* driver;
	double distance;
};

//Similar, but for pedestrians
struct NearestPedestrian {
	NearestPedestrian() : distance(5000) {}

	//TODO: This is probably not needed. We should really set "distance" to DOUBLE_MAX.
	bool exists() { return distance < 5000; }

	double distance;
};


///Simple struct to hold parameters which only exist for a single update tick.
//NOTE: Constructor is currently implemented in Driver.cpp. Feel free to shuffle this around if you like.
struct DriverUpdateParams : public UpdateParams {
	DriverUpdateParams(boost::mt19937& gen) : UpdateParams(gen) {}

	virtual void reset(frame_t frameNumber, unsigned int currTimeMS, const Driver& owner);

	const Lane* currLane;  //TODO: This should really be tied to PolyLineMover, but for now it's not important.
	size_t currLaneIndex; //Cache of currLane's index.
	size_t fromLaneIndex; //for lane changing model
	const Lane* leftLane;
	const Lane* rightLane;

	double currSpeed;

	double currLaneOffset;
	double currLaneLength;
	bool isTrafficLightStop;
	double trafficSignalStopDistance;
	double elapsedSeconds;

	double perceivedFwdVelocity;
	double perceivedLatVelocity;

	double perceivedFwdVelocityOfFwdCar;
	double perceivedLatVelocityOfFwdCar;
	double perceivedAccelerationOfFwdCar;
	double perceivedDistToFwdCar;

	//Nearest vehicles in the current lane, and left/right (including fwd/back for each).
	//Nearest vehicles' distances are initialized to threshold values.
	NearestVehicle nvFwd;
	NearestVehicle nvBack;
	NearestVehicle nvLeftFwd;
	NearestVehicle nvLeftBack;
	NearestVehicle nvRightFwd;
	NearestVehicle nvRightBack;

	NearestPedestrian npedFwd;

	double laneChangingVelocity;

	bool isCrossingAhead;
	bool isApproachingToIntersection;
	int crossingFwdDistance;

	//Related to our car following model.
	double space;
	double a_lead;
	double v_lead;
	double space_star;
	double distanceToNormalStop;

	//Related to our lane changing model.
	double dis2stop;
	bool isWaiting;

	//Handles state information
	bool justChangedToNewSegment;
	DPoint TEMP_lastKnownPolypoint;
	bool justMovedIntoIntersection;
	double overflowIntoIntersection;
};



}
