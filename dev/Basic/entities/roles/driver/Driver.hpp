/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <math.h>
#include <set>

#include "entities/roles/Role.hpp"
#include "buffering/Buffered.hpp"
#include "geospatial/StreetDirectory.hpp"
#include "perception/FixedDelayed.hpp"
#include "util/DynamicVector.hpp"

#include "CarFollowModel.hpp"
#include "LaneChangeModel.hpp"
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
class Vehicle;
class DPoint;


class Driver : public sim_mob::Role {

//Constructor and overridden methods.
public:
	Driver (Agent* parent);			//to initiate
	virtual void update(frame_t frameNumber);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();


//Basic data
private:
	//Pointer to the vehicle this driver is controlling.
	Vehicle* vehicle;

	//Update models
	LaneChangeModel* lcModel;
	CarFollowModel* cfModel;

	//More update methods
	void update_first_frame(UpdateParams& params, frame_t frameNumber);
	void update_general(UpdateParams& params, frame_t frameNumber);

	//Sample stored data which takes reaction time into account.
	const static size_t reactTime = 1500; //1.5 seconds
	FixedDelayed<DPoint*> perceivedVelocity;
	FixedDelayed<Point2D*> perceivedVelocityOfFwdCar;
	FixedDelayed<centimeter_t> perceivedDistToFwdCar;

	//absolute position of the target start point on the next link
	//used for intersection driving behavior
	int xPos_nextLink;
	int yPos_nextLink;

	double crossingFarX;
	double crossingFarY;
	double crossingNearX;
	double crossingNearY;

	Point2D origin;
	Point2D goal;
	const Node* destNode;				//first, assume that each vehicle moves towards a goal
	const Node* originNode;				//when a vehicle reaches its goal, it will return to origin and moves to the goal again
	bool firstFrameTick;			//to check if the origin has been set

	double maxLaneSpeed;

public:
	//int getTimeStep() const {return timeStep;}
	void assignVehicle(Vehicle* v) {vehicle = v;}

	//for coordinate transform
	void setParentBufferedData();			///<set next data to parent buffer data
	void output(UpdateParams& p, frame_t frameNumber);

	/****************IN REAL NETWORK****************/
private:
	static void check_and_set_min_car_dist(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other);

	const Link* desLink;
    double currLinkOffset;

	size_t targetLaneIndex;
	StreetDirectory::LaneAndIndexPair laneAndIndexPair;
	const std::vector<sim_mob::Point2D>* desLanePolyLine;

	Point2D desPolyLineStart;
	Point2D desPolyLineEnd;


	DynamicVector intersectionTrajectory;
	double intersectionDistAlongTrajectory;

	//Point2D entryPoint; //entry point for crossing intersection
	//int xTurningStart;
	//int yTurningStart;
	//double xDirection_entryPoint;
	//double yDirection_entryPoint;
	//int disToEntryPoint;
	bool isCrossingAhead;
	bool closeToCrossing;

	//Parameters relating to the next Link we plan to move to after an intersection.
	const Link* nextLink;
	const Lane* nextLaneInNextLink;
	//bool nextIsForward;
	bool isReachGoal;

	const int distanceInFront;
	const int distanceBehind;

public:
	Buffered<const Lane*> currLane_;
	Buffered<double> currLaneOffset_;
	Buffered<double> currLaneLength_;
	Buffered<bool> isInIntersection;

public:
	const Vehicle* getVehicle() const {return vehicle;}

private:
	bool isLeaveIntersection() const;
	bool isCloseToLinkEnd(UpdateParams& p);
	bool isPedetrianOnTargetCrossing();
	void chooseNextLaneForNextLink(UpdateParams& p);
	void calculateIntersectionTrajectory();
	int disToObstacle(unsigned obstacle_offset);
	void setOrigin(UpdateParams& p);

	//A bit verbose, but only used in 1 or 2 places.
	void syncCurrLaneCachedInfo(UpdateParams& p);
	void justLeftIntersection(UpdateParams& p);
	void updateAdjacentLanes(UpdateParams& p);
	void updateAcceleration(double newFwdAcc);
	void updateVelocity();
	void updatePositionOnLink(UpdateParams& p);
	void setBackToOrigin();

	void updateNearbyAgents(UpdateParams& params);
	void updateNearbyDriver(UpdateParams& params, const sim_mob::Person* other, const sim_mob::Driver* other_driver);
	void updateNearbyPedestrian(UpdateParams& params, const sim_mob::Person* other, const sim_mob::Pedestrian* pedestrian);

	void updateCurrLaneLength(UpdateParams& p);
	void updateDisToLaneEnd();
	void updatePositionDuringLaneChange(UpdateParams& p);
	void updateTrafficSignal();

	void trafficSignalDriving(UpdateParams& p);
	void intersectionDriving(UpdateParams& p);
	void linkDriving(UpdateParams& p);

	void initializePath();
	void findCrossing(UpdateParams& p);


	/***********SOMETHING BIG BROTHER CAN RETURN*************/
private:
	const Pedestrian* CFP;
	const Pedestrian* LFP;
	const Pedestrian* RFP;


	/***********FOR DRIVING BEHAVIOR MODEL**************/
private:
	double targetSpeed;			//the speed which the vehicle is going to achieve

	/**************BEHAVIOR WHEN APPROACHING A INTERSECTION***************/
public:
	void updateAngle(UpdateParams& p);
	void intersectionVelocityUpdate();
	void modifyPosition();
	void IntersectionDirectionUpdate();
	void UpdateNextLinkLane();
	bool isReachCrosswalk();

	//This always returns the lane we are moving towards; regardless of if we've passed the
	//  halfway point or not.
	LANE_CHANGE_SIDE getCurrLaneChangeDirection() const;

	//This, however, returns where we are relative to the center of our own lane.
	// I'm sure we can do this in a less confusion fashion later.
	LANE_CHANGE_SIDE getCurrLaneSideRelativeToCenter() const;

private:
	//The current traffic signal in our Segment. May be null.
	const Signal* trafficSignal;


};



}
