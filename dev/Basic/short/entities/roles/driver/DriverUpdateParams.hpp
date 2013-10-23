//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/settings/DisableMPI.h"

#include "entities/UpdateParams.hpp"
#include "geospatial/Lane.hpp"
#include "util/DynamicVector.hpp"
#include <boost/random.hpp>
#include "util/LangHelpers.hpp"
#include "entities/signal/Signal.hpp"

namespace sim_mob
{

//Forward declarations
class Lane;
class Driver;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif


enum LANE_CHANGE_MODE {	//as a mask
	DLC = 0,
	MLC = 2
};


//Struct for holding data about the "nearest" vehicle.
struct NearestVehicle {
	NearestVehicle() : driver(nullptr), distance(50000) {}

	//TODO: This is probably not needed. We should really set "distance" to DOUBLE_MAX.
	bool exists() const { return distance < 5000; }
	const Driver* driver;
	double distance;
};

//Similar, but for pedestrians
struct NearestPedestrian {
	NearestPedestrian() : distance(50000) {}

	//TODO: This is probably not needed. We should really set "distance" to DOUBLE_MAX.
	bool exists() { return distance < 5000; }

	double distance;
};


///Simple struct to hold parameters which only exist for a single update tick.
/// \author Wang Xinyuan
/// \author Li Zhemin
/// \author Seth N. Hetu
///NOTE: Constructor is currently implemented in Driver.cpp. Feel free to shuffle this around if you like.
struct DriverUpdateParams : public UpdateParams {
	DriverUpdateParams() : UpdateParams() {}
	explicit DriverUpdateParams(boost::mt19937& gen) : UpdateParams(gen) ,nextLaneIndex(0){}

	virtual void reset(timeslice now, const Driver& owner);

	const Lane* currLane;  //TODO: This should really be tied to PolyLineMover, but for now it's not important.
	size_t currLaneIndex; //Cache of currLane's index.
	size_t nextLaneIndex; //for lane changing model
	const Lane* leftLane;
	const Lane* rightLane;
	const Lane* leftLane2; //the second left lane
	const Lane* rightLane2;

	double currSpeed;

	double currLaneOffset;
	double currLaneLength;
	double trafficSignalStopDistance;
	double elapsedSeconds;
	sim_mob::TrafficColor trafficColor;


	double perceivedFwdVelocity;
	double perceivedLatVelocity;

	double perceivedFwdVelocityOfFwdCar;
	double perceivedLatVelocityOfFwdCar;
	double perceivedAccelerationOfFwdCar;
	double perceivedDistToFwdCar;
	double perceivedDistToTrafficSignal;

	DriverUpdateParams& operator=(DriverUpdateParams rhs)
	{
		currLane = rhs.currLane;
		currLaneIndex = rhs.currLaneIndex;
		nextLaneIndex = rhs.nextLaneIndex;

		return *this;
	}

	sim_mob::TrafficColor perceivedTrafficColor;
	LANE_CHANGE_SIDE turningDirection;

	//Nearest vehicles in the current lane, and left/right (including fwd/back for each).
	//Nearest vehicles' distances are initialized to threshold values.
	bool isAlreadyStart;
	bool isBeforIntersecton;
	NearestVehicle nvFwdNextLink;
	NearestVehicle nvFwd;
	NearestVehicle nvBack;
	NearestVehicle nvLeftFwd;
	NearestVehicle nvLeftBack;
	NearestVehicle nvRightFwd;
	NearestVehicle nvRightBack;
	NearestVehicle nvLeftFwd2; //the second adjacent lane
	NearestVehicle nvLeftBack2;
	NearestVehicle nvRightFwd2;
	NearestVehicle nvRightBack2;



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

public:
#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const DriverUpdateParams* params);

	static void unpack(UnPackageUtils& unpackage, DriverUpdateParams* params);
#endif
};


}
