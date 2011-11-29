/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <math.h>
#include <set>

#include "entities/roles/Role.hpp"
#include "buffering/Buffered.hpp"
#include "geospatial/StreetDirectory.hpp"
#include "perception/FixedDelayed.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "util/DynamicVector.hpp"

#include "CarFollowModel.hpp"
#include "LaneChangeModel.hpp"
#include "IntersectionDrivingModel.hpp"
#include "UpdateParams.hpp"


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
	Driver (Agent* parent);			//to initiate
	virtual void update(frame_t frameNumber);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

//Buffered data
public:
	Buffered<const Lane*> currLane_;
	Buffered<double> currLaneOffset_;
	Buffered<double> currLaneLength_;
	Buffered<bool> isInIntersection;

//Basic data
private:
	//Pointer to the vehicle this driver is controlling.
	Vehicle* vehicle;

	//Update models
	LaneChangeModel* lcModel;
	CarFollowModel* cfModel;
	IntersectionDrivingModel* intModel;

	//Sample stored data which takes reaction time into account.
	const static size_t reactTime = 1500; //1.5 seconds
	FixedDelayed<DPoint*> perceivedVelocity;
	FixedDelayed<Point2D*> perceivedVelocityOfFwdCar;
	FixedDelayed<centimeter_t> perceivedDistToFwdCar;

	NodePoint origin;
	NodePoint goal;    //first, assume that each vehicle moves towards a goal
	bool firstFrameTick;			//to check if the origin has been set

	double maxLaneSpeed;

public:
	//for coordinate transform
	void setParentBufferedData();			///<set next data to parent buffer data
	void output(UpdateParams& p, frame_t frameNumber);

	/****************IN REAL NETWORK****************/
private:
	static void check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other);

	//More update methods
	void update_first_frame(UpdateParams& params, frame_t frameNumber);    ///<Called the first time a frame after start_time is reached.
	void update_sensors(UpdateParams& params, frame_t frameNumber);        ///<Called to update things we _sense_, like nearby vehicles.
	void update_movement(UpdateParams& params, frame_t frameNumber);       ///<Called to move vehicles forward.
	void update_post_movement(UpdateParams& params, frame_t frameNumber);  ///<Called to deal with the consequences of moving forwards.

	const Link* desLink;
    double currLinkOffset;

	size_t targetLaneIndex;

	//Driving through an intersection on a given trajectory.
	//TODO: A bit buggy.
	//DynamicVector intersectionTrajectory;
	//double intersectionDistAlongTrajectory;

	//Parameters relating to the next Link we plan to move to after an intersection.
	const Link* nextLink;
	const Lane* nextLaneInNextLink;

public:
	//TODO: This may be risky, as it exposes non-buffered properties to other vehicles.
	const Vehicle* getVehicle() const {return vehicle;}

	//This is probably ok.
	const double getVehicleLength() const { return vehicle->length; }

private:
	bool isPedestrianOnTargetCrossing() const;
	void chooseNextLaneForNextLink(UpdateParams& p);
	void calculateIntersectionTrajectory(DPoint movingFrom, double overflow);
	void setOrigin(UpdateParams& p);

	//A bit verbose, but only used in 1 or 2 places.
	void syncCurrLaneCachedInfo(UpdateParams& p);
	void justLeftIntersection(UpdateParams& p);
	void updateAdjacentLanes(UpdateParams& p);
	void updateVelocity();
	double updatePositionOnLink(UpdateParams& p);
	void setBackToOrigin();

	void updateNearbyAgents(UpdateParams& params) const;
	void updateNearbyDriver(UpdateParams& params, const sim_mob::Person* other, const sim_mob::Driver* other_driver) const;
	void updateNearbyPedestrian(UpdateParams& params, const sim_mob::Person* other, const sim_mob::Pedestrian* pedestrian) const;

	//void updateCurrLaneLength(UpdateParams& p);
	void updateDisToLaneEnd();
	void updatePositionDuringLaneChange(UpdateParams& p, LANE_CHANGE_SIDE relative);
	void saveCurrTrafficSignal();

	void setTrafficSignalParams(UpdateParams& p) const;
	void intersectionDriving(UpdateParams& p);
	double linkDriving(UpdateParams& p);

	void initializePath();
	void findCrossing(UpdateParams& p);


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
	static const bool DebugOn;
	mutable std::stringstream DebugStream;


};



}
