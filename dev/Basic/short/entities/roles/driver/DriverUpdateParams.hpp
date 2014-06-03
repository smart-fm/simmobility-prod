//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/settings/DisableMPI.h"

#include "entities/UpdateParams.hpp"
#include "entities/models/LaneChangeModel.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadSegment.hpp"
#include "util/DynamicVector.hpp"
#include <boost/random.hpp>
#include "util/LangHelpers.hpp"
#include "entities/signal/Signal.hpp"
//#include "entities/roles/driver/Driver.hpp"

namespace sim_mob
{

//Forward declarations
class Lane;
class Driver;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif



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
class DriverUpdateParams : public UpdateParams {
public:
	DriverUpdateParams() : UpdateParams() ,status(0),yieldTime(0,0),lcTimeTag(0){}
	explicit DriverUpdateParams(boost::mt19937& gen) : UpdateParams(gen) ,nextLaneIndex(0),isTargetLane(true),status(0),yieldTime(0,0),lcTimeTag(0){}

	virtual void reset(timeslice now, const Driver& owner);

	const Lane* currLane;  //TODO: This should really be tied to PolyLineMover, but for now it's not important.
	size_t currLaneIndex; //Cache of currLane's index.
	size_t nextLaneIndex; //for lane changing model
	const Lane* leftLane;
	const Lane* rightLane;
	const Lane* leftLane2; //the second left lane
	const Lane* rightLane2;

	double currSpeed;
	double desiredSpeed;

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
		status = rhs.status;

		return *this;
	}

	sim_mob::TrafficColor perceivedTrafficColor;
	LANE_CHANGE_SIDE turningDirection;

	TARGET_GAP targetGap;
	bool isMLC;
	LANE_CHANGE_MODE lastChangeMode;
	/// record last lane change decision
	/// both lc model and driverfacet can set this value
	LANE_CHANGE_SIDE lastDecision;

	//Nearest vehicles in the current lane, and left/right (including fwd/back for each).
	//Nearest vehicles' distances are initialized to threshold values.
	bool isAlreadyStart;
	bool isBeforIntersecton;
	// used to check vh opposite intersection
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
	// used to check vh when do acceleration merging
	NearestVehicle nvLeadFreeway; // lead vh on freeway segment,used when subject vh on ramp
	NearestVehicle nvLagFreeway;// lag vh on freeway,used when subject vh on ramp

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

	Driver* driver;

	/// if current lane connect to target segment
	/// assign in driverfact
	bool isTargetLane;

	/// record last calculated acceleration
	/// dont reset
	double lastAcc;

	/**
	 *  /brief add one kind of status to the vh
	 *  /param new state
	 */
	void setStatus(unsigned int s);
	/**
	 *  /brief get status of the vh
	 *  /return state
	 */
	unsigned int getStatus() { return status; }
	unsigned int getStatus(unsigned int mask) {
		return (status & mask);
	}
	/**
	 *  /brief remove the status from the vh
	 *  /return state
	 */
	void unsetStatus(unsigned int s);

	unsigned int status;	// current status indicator
	unsigned int flags;	// additional indicator for internal use

	void toggleFlag(unsigned int flag) {
	flags ^= flag;
	}
	unsigned int flag(unsigned int mask = 0xFFFFFFFF) {
	return (flags & mask);
	}
	void setFlag(unsigned int s) {
	flags |= s;
	}
	void unsetFlag(unsigned int s) {
	flags &= ~s;
	}

	/// decision timer (second)
	/// count down in DriverMovement
	double cftimer;
	double nextStepSize;
	double getNextStepSize() { return nextStepSize; }

	const RoadSegment* nextLink();

	double maxAcceleration;
	double normalDeceleration;
	double maxDeceleration;

	bool willYield(unsigned int reason);
	timeslice yieldTime;	// time to start yielding
	double lcTimeTag;		// time changed lane , ms
	vector<double> nosingParams;
	double lcMaxNosingTime;
public:
#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const DriverUpdateParams* params);

	static void unpack(UnPackageUtils& unpackage, DriverUpdateParams* params);
#endif
};


}
