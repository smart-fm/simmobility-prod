#pragma once

#include "Role.hpp"

namespace sim_mob
{



/**
 * A Person in the Driver role is navigating roads and intersections.
 *
 *
 * - wangxy
 *
 * Till now, the vehicle can make acceleration decision and do simple lane changing decision.
 * When it changes lane, it will slowly move to the adjacent lane and can avoid crash,
 * which is different from the model of MITSIMLab(just directly move to the adjacent lane in next time step).
 * Since the big brother hasn't been finished, I also add many parameters in agent class(see Agent.hpp),
 * helping each driver know enough information of other vehicles.
 *
 * Now that the car just go in a direct road with 3 lanes. They will move in x direction.
 * When they want to change lane, they will move in y direction.
 * All of these are temporarily made to check if the driving behavior can function,
 * sine network and big brother haven't been finished.
 * So many of the details of the functions may change when network and big brother
 * introduced into this program.
 *
 * - wangxy
 */

class Driver : public sim_mob::Role {
public:
	Driver (Agent* parent);			//to initiate
	virtual void update(frame_t frameNumber);


/**
 * First of all, I'd like to introduce some constant parameters here for the class to use.
 * Maybe many of them will be moved to the database later.
 * So Just don't be surprised of so many constant parameters below:
 *
 * NOTE: I made these static. ~Seth
 */
private:
	static const double MAX_ACCELERATION		=	+10.0;
	static const double MAX_DECELERATION		=	-10.0;

	//Something I have to define
	static const double maxLaneSpeed[];
	static const double FLT_EPSILON			=	0.0001;		//the smallest double
	static const double MAX_NUM				=	30000;			//regard as infinity
	static const double hBufferUpper			=	1;				//upper threshold of headway
	static const double hBufferLower			=	0.5;			//lower threshold of headway

	//Parameters represent the location of the vehicle on the road
	//Since the classes of the network haven't be finished, I will use simple type of parameters instead to represent the networks.
	static const double lane[];	//the y position of 3 lanes
	static const double laneWidth				=	20;
	static const double VelOfLaneChanging		=	4;				//assume that each car use the same speed to move in y direction to change lane


	/**********BASIC DATA*************/
private:
	//Movement-related variables
	double timeStep;			//time step size of simulation
	double speed;
	double xPos;
	double yPos;
	double xVel;
	double yVel;
	double acc;
	double xAcc;
	double yAcc;
	Point2D goal;				//first, assume that each vehicle moves towards a goal
	Point2D origin;				//when a vehicle reaches its goal, it will return to origin and moves to the goal again
	bool isGoalSet;				//to check if the goal has been set
	bool isOriginSet;			//to check if the origin has been set
	double length;				//length of the vehicle
	double width;				//width of the vehicle

	double maxAcceleration;
	double normalDeceleration;
	double maxDeceleration;
	double distanceToNormalStop;

public:
	double getTimeStep(){return timeStep;}
	double getSpeed(){return speed;}
	double getLength(){return length;}

	double getMaxAcceleration(){return maxAcceleration;}
	double getNormalDeceleration(){return normalDeceleration;}
	double getMaxDeceleration(){return maxDeceleration;}
	double getDistanceToNormalStop(){return distanceToNormalStop;}
	double getDistance();


	/***********SOMETHING BIG BROTHER CAN RETURN*************/
private:
	Agent* leadingDriver;					//a pointer pointing the leading driver
public:
	void updateLeadingDriver();				//this may be a function of big brother later
	Agent* getLeadingDriver(){return leadingDriver;}
	int getLane();				//return the ID of the lane where vehicle is on


	/***********FOR DRIVING BEHAVIOR MODEL**************/
	//parameters
private:
	double targetSpeed;			//the speed which the vehicle is going to achieve
	double space;				//the distance between subject vehicle to leading vehicle
	double headway;				//distance/speed
	double space_star;			//the distance which leading vehicle will move in next time step
	double dv;					//the difference of subject vehicle's speed and leading vehicle's speed
	double a_lead;				//the acceleration of leading vehicle
	double v_lead;				//the speed of leading vehicle

	//for acceleration decision
public:
	void makeAcceleratingDecision();				//decide acc
	double breakToTargetSpeed();					//return the acc to a target speed within a specific distance
	double accOfEmergencyDecelerating();			//when headway < lower threshold, use this function
	double accOfCarFollowing();						//when lower threshold < headway < upper threshold, use this function
	double accOfMixOfCFandFF();						//when upper threshold < headway, use this funcion
	double accOfFreeFlowing();						//is a part of accofMixOfCFandFF
	double getTargetSpeed(){return targetSpeed;}

	//for lane changing decision
private:
	Agent* LF;				//pointer pointing to the vehicle in the left lane and in front of the vehicle in the smallest distance
	Agent* LB;				//left lane, behind, closest
	Agent* RF;				//right lane, front, closest
	Agent* RB;				//right lane, behind, closest
	double changeDecision;	//1 for right, -1 for left, 0 for current
	bool ischanging;		//is the vehicle is changing the lane
	bool isback;			//is the vehicle get back to the lane to avoid crash
	int fromLane;			//during lane changing, the lane the vehicle leaves
	int toLane;				//during lane changing, the lane the vehicle approaches
public:
	Agent* getNextForBDriver(bool isLeft,bool isFront);	//for updating LF LB RF RB
	int gapAcceptance();				//check if the gap of the left lane and the right lane is available
										//return 2 for both, 1 for right, -1 for right, 0 for neither
	double makeLaneChangingDecision();		//to decide which lane to move, the returns are same as above
	void excuteLaneChanging();			//to execute the lane changing, meanwhile, check if crash will happen and avoid it
	bool checkForCrash();				//to check if the crash may happen


	/*****************FUNCTIONS FOR UPDATING****************/
private:
	bool isGoalReached();
	void setGoal();
	void setOrigin();
	void updateAcceleration();
	void updateVelocity();
	void updatePosition();

};



}
