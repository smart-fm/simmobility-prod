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

//TODO: Once the new signal class is stabilized, replace this include with a forward declaration:
#include "entities/signal_transitional.hpp"

namespace sim_mob
{

//Forward declarations
class Pedestrian;
//class Signal;
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
	const static int distanceInFront = 3000;
	const static int distanceBehind = 500;
	const static int maxVisibleDis = 5000;



	Driver(Person* parent, sim_mob::MutexStrategy mtxStrat);
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

	//This should be done through the Role class itself; for now, I'm just forcing
	//  it so that we can get the mid-term working. ~Seth
	virtual Vehicle* getResource() { return vehicle; }


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

	size_t reacTime;
	FixedDelayed<double> *perceivedFwdVel;
	FixedDelayed<double> *perceivedFwdAcc;
	FixedDelayed<double> *perceivedVelOfFwdCar;
	FixedDelayed<double> *perceivedAccOfFwdCar;
	FixedDelayed<double> *perceivedDistToFwdCar;
#ifdef SIMMOB_NEW_SIGNAL
	FixedDelayed<sim_mob::TrafficColor> *perceivedTrafficColor;
#else
	FixedDelayed<Signal::TrafficColor> *perceivedTrafficColor;
#endif
	FixedDelayed<double> *perceivedDistToTrafficSignal;

	NodePoint origin;
	NodePoint goal;    //first, assume that each vehicle moves towards a goal



	double disToFwdVehicleLastFrame; //to find whether vehicle is going to crash in current frame.
	                                     //so distance in last frame need to be remembered.


public:
	double maxLaneSpeed;
	//for coordinate transform
	void setParentBufferedData();			///<set next data to parent buffer data

	Agent* getDriverParent(const Driver *self) { return self->parent; }
private:
	static void check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other);

	//More update methods
	//void update_first_frame(DriverUpdateParams& params, frame_t frameNumber);    ///<Called the first time a frame after start_time is reached.
	bool update_sensors(DriverUpdateParams& params, frame_t frameNumber);        ///<Called to update things we _sense_, like nearby vehicles.
	bool update_movement(DriverUpdateParams& params, frame_t frameNumber);       ///<Called to move vehicles forward.
	bool update_post_movement(DriverUpdateParams& params, frame_t frameNumber);  ///<Called to deal with the consequences of moving forwards.

    double currLinkOffset;
	size_t targetLaneIndex;

	const Lane* nextLaneInNextLink;

public:
	//TODO: This may be risky, as it exposes non-buffered properties to other vehicles.
	const Vehicle* getVehicle() const {return vehicle;}

	//This is probably ok.
	const double getVehicleLength() const { return vehicle->length; }

	void updateAdjacentLanes(DriverUpdateParams& p);
	void updatePositionDuringLaneChange(DriverUpdateParams& p, LANE_CHANGE_SIDE relative);


protected:
	virtual double updatePositionOnLink(DriverUpdateParams& p);
	virtual double linkDriving(DriverUpdateParams& p);

	//TODO: Eventually move these into the medium/ folder.
	sim_mob::Vehicle* initializePath(bool allocateVehicle);
	void initializePathMed();

	void resetPath(DriverUpdateParams& p);
	void setOrigin(DriverUpdateParams& p);

	//Helper: for special strings
	void initLoopSpecialString(std::vector<WayPoint>& path, const std::string& value);
	void initTripChainSpecialString(const std::string& value);
	NearestVehicle & nearestVehicle(DriverUpdateParams& p);

	void perceivedDataProcess(NearestVehicle & nv, DriverUpdateParams& params);



private:
	bool AvoidCrashWhenLaneChanging(DriverUpdateParams& p);
	bool isCloseToLinkEnd(DriverUpdateParams& p) const;
	bool isPedestrianOnTargetCrossing() const;
	void chooseNextLaneForNextLink(DriverUpdateParams& p);
	void calculateIntersectionTrajectory(DPoint movingFrom, double overflow);

	//A bit verbose, but only used in 1 or 2 places.
	void syncCurrLaneCachedInfo(DriverUpdateParams& p);
	void justLeftIntersection(DriverUpdateParams& p);

	void updateVelocity();
	void setBackToOrigin();

	void updateNearbyAgents(DriverUpdateParams& params);
	void updateNearbyDriver(DriverUpdateParams& params, const sim_mob::Person* other, const sim_mob::Driver* other_driver);
	void updateNearbyPedestrian(DriverUpdateParams& params, const sim_mob::Person* other, const sim_mob::Pedestrian* pedestrian);

	//void updateCurrLaneLength(DriverUpdateParams& p);
	void updateDisToLaneEnd();

	void saveCurrTrafficSignal();

	void setTrafficSignalParams(DriverUpdateParams& p);
	void intersectionDriving(DriverUpdateParams& p);


	void findCrossing(DriverUpdateParams& p);


	/***********FOR DRIVING BEHAVIOR MODEL**************/
private:

	/**************BEHAVIOR WHEN APPROACHING A INTERSECTION***************/
public:
	double targetSpeed;			//the speed which the vehicle is going to achieve

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
