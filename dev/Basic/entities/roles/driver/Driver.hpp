/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <math.h>
#include <set>

#include "GenConfig.h"

#include "entities/roles/Role.hpp"
#include "buffering/Shared.hpp"
#include "geospatial/StreetDirectory.hpp"
#include "perception/FixedDelayed.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "util/DynamicVector.hpp"

#include "CarFollowModel.hpp"
#include "LaneChangeModel.hpp"
#include "IntersectionDrivingModel.hpp"
#include "DriverUpdateParams.hpp"

namespace sim_mob
{

//Forward declarations
class Pedestrian;
class Signal;
class Link;
class RoadSegment;
class Lane;
class Node;
class MultiNode;
class DPoint;
class UpdateParams;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif


/**
 * \author Wang Xinyuan
 * \author Li Zhemin
 * \author Runmin Xu
 * \author Seth N. Hetu
 * \author Luo Linbo
 * \author LIM Fung Chai
 * \author Zhang Shuai
 * \author Xu Yan
 */
class Driver : public sim_mob::Role {
//Internal classes
private:
	//Helper class for grouping a Node and a Point2D together.
	class NodePoint {
	public:
		Point2D point;
		const Node* node;
		NodePoint() : point(0,0), node(nullptr) {}
	};


//Constructor and overridden methods.
public:
	Driver(Person* parent, sim_mob::MutexStrategy mtxStrat, unsigned int reacTime_LeadingVehicle, unsigned int reacTime_SubjectVehicle, unsigned int reacTime_Gap);		//to initiate
	virtual ~Driver();

	//Virtual implementations
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_med(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(frame_t frameNumber);

	virtual UpdateParams& make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

//Buffered data
public:
	Shared<const Lane*> currLane_;
	Shared<double> currLaneOffset_;
	Shared<double> currLaneLength_;
	Shared<bool> isInIntersection;

	//need to store these values in the double buffer, because it is needed by other drivers.
	Shared<double> latMovement;
	Shared<double> fwdVelocity;
	Shared<double> latVelocity;
	Shared<double> fwdAccel;
	Shared<LANE_CHANGE_SIDE> turningDirection;

//Basic data
protected:
	//unsigned int currTimeMS;
	//Pointer to the vehicle this driver is controlling.
	Vehicle* vehicle;

protected:
	//Temporary variable which will be flushed each time tick. We save it
	// here to avoid constantly allocating and clearing memory each time tick.
	DriverUpdateParams params;

	//Update models
	LaneChangeModel* lcModel;
	CarFollowModel* cfModel;
	IntersectionDrivingModel* intModel;

private:
	//Sample stored data which takes reaction time into account.
	unsigned int reacTime_LeadingVehicle;
	unsigned int reacTime_SubjectVehicle;
	unsigned int reacTime_Gap;
	FixedDelayed<DPoint*> perceivedVelocity;
	FixedDelayed<DPoint*> perceivedVelocityOfFwdCar;
	FixedDelayed<double> perceivedAccelerationOfFwdCar;
	FixedDelayed<centimeter_t> perceivedDistToFwdCar;
	FixedDelayed<double> perceivedTrafficSignalStop;

	NodePoint origin;
	NodePoint goal;    //first, assume that each vehicle moves towards a goal


	double maxLaneSpeed;

public:
	//for coordinate transform
	void setParentBufferedData();			///<set next data to parent buffer data
	//void output(frame_t frameNumber);

	/****************IN REAL NETWORK****************/
private:
	static void check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other);

	//More update methods
	//void update_first_frame(DriverUpdateParams& params, frame_t frameNumber);    ///<Called the first time a frame after start_time is reached.
	bool update_sensors(DriverUpdateParams& params, frame_t frameNumber);        ///<Called to update things we _sense_, like nearby vehicles.
	bool update_movement(DriverUpdateParams& params, frame_t frameNumber);       ///<Called to move vehicles forward.
	bool update_post_movement(DriverUpdateParams& params, frame_t frameNumber);  ///<Called to deal with the consequences of moving forwards.

//	const Link* desLink;
    double currLinkOffset;

	size_t targetLaneIndex;

	//Driving through an intersection on a given trajectory.
	//TODO: A bit buggy.
	//DynamicVector intersectionTrajectory;
	//double intersectionDistAlongTrajectory;

	//Parameters relating to the next Link we plan to move to after an intersection.
//	const Link* nextLink;
	const Lane* nextLaneInNextLink;

public:
	//TODO: This may be risky, as it exposes non-buffered properties to other vehicles.
	const Vehicle* getVehicle() const {return vehicle;}

	//This is probably ok.
	const double getVehicleLength() const { return vehicle->length; }

protected:
	virtual double updatePositionOnLink(DriverUpdateParams& p);
	void initializePath();
	void resetPath(DriverUpdateParams& p);
	void setOrigin(DriverUpdateParams& p);

	//Helper: for special strings
	void initLoopSpecialString(std::vector<WayPoint>& path, const std::string& value);
	void initTripChainSpecialString(const std::string& value);

private:
	NearestVehicle & nearestVehicle(DriverUpdateParams& p);
	bool AvoidCrashWhenLaneChanging(DriverUpdateParams& p);
	bool isCloseToLinkEnd(DriverUpdateParams& p) const;
	bool isPedestrianOnTargetCrossing() const;
	void chooseNextLaneForNextLink(DriverUpdateParams& p);
	void calculateIntersectionTrajectory(DPoint movingFrom, double overflow);

	//A bit verbose, but only used in 1 or 2 places.
	void syncCurrLaneCachedInfo(DriverUpdateParams& p);
	void justLeftIntersection(DriverUpdateParams& p);
	void updateAdjacentLanes(DriverUpdateParams& p);
	void updateVelocity();
	void setBackToOrigin();

	void updateNearbyAgents(DriverUpdateParams& params);
	void updateNearbyDriver(DriverUpdateParams& params, const sim_mob::Person* other, const sim_mob::Driver* other_driver);
	void updateNearbyPedestrian(DriverUpdateParams& params, const sim_mob::Person* other, const sim_mob::Pedestrian* pedestrian);

	//void updateCurrLaneLength(DriverUpdateParams& p);
	void updateDisToLaneEnd();
	void updatePositionDuringLaneChange(DriverUpdateParams& p, LANE_CHANGE_SIDE relative);
	void saveCurrTrafficSignal();

	void setTrafficSignalParams(DriverUpdateParams& p);
	void intersectionDriving(DriverUpdateParams& p);
	double linkDriving(DriverUpdateParams& p);

	void findCrossing(DriverUpdateParams& p);

	void perceivedDataProcess(NearestVehicle & nv, DriverUpdateParams& params);

	/***********FOR DRIVING BEHAVIOR MODEL**************/
private:
	double targetSpeed;			//the speed which the vehicle is going to achieve

	/**************BEHAVIOR WHEN APPROACHING A INTERSECTION***************/
public:
	void intersectionVelocityUpdate();

	//This always returns the lane we are moving towards; regardless of if we've passed the
	//  halfway point or not.
	LANE_CHANGE_SIDE getCurrLaneChangeDirection() const;

	//This, however, returns where we are relative to the center of our own lane.
	// I'm sure we can do this in a less confusion fashion later.
	LANE_CHANGE_SIDE getCurrLaneSideRelativeToCenter() const;

private:
	//The current traffic signal in our Segment. May be null.
	const Signal* trafficSignal;

	//For generating a debugging trace
	mutable std::stringstream DebugStream;

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif

};



}
