#pragma once

#include <limits>

#include "Role.hpp"

namespace sim_mob
{

enum LANE_SIDE {
	LSIDE_LEFT = 1,
	LSIDE_RIGHT = 2
};

enum LANE_CHANGE_MODE {	//as a mask
	DLC = 0,
	MLC = 2
};

//bad area where vehicle can not go
struct badArea{
	double startX;
	double endX;
	int lane;
};

//simple link structure
struct link_{
	int ID;
	double startX;
	double startY;
	double endX;
	double endY;
	double laneWidth;
	int laneNum;
};

/**
 * A Person in the Driver role is navigating roads and intersections.
 *
 */

/*
 * Till now, the driver can drive vehicles on a test link(simple structure defined below).
 * They can make accelerating decision based on MITSIMLab model and do mandatory and discretionary
 * lane changing based on model simplified from MITSIMLab model.
 * The structure 'bad areas' is used to test if the mandatory lane changing can function, which will
 * no be introduced into final code in the future.
 *
 * -wangxy
 * */

/*
 * 26/7/2011 update info:
 * 1.Now drivers can transform absolute coordinate to relative coordinate and vice versa.
 *  In order to merge driving behavior model with sub-system network, drivers should know
 *  relative coordinate of the current link.
 *
 * \ToDo:
 * 1.Working on behavior when driving from one link to another link. Especially driving behavior
 *  when passing an intersection.
 *
 *  -wangxy
 * */

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
	static const double MAX_ACCELERATION		=	+150.0;
	static const double MAX_DECELERATION		=	-150.0;

	//Something I have to define
	static const double maxLaneSpeed[];
	static const double FLT_EPSILON				=	0.0001;	//the smallest double
	static const double MAX_NUM;							//regard as infinity
	static const double hBufferUpper			=	1.6;	//upper threshold of headway
	static const double hBufferLower			=	0.8;	//lower threshold of headway

	//parameters(refer to MITSIMLab data files)
	static const double CF_parameters[2][6];	//Car Following parameters
	static const double GA_parameters[4][9];	//Gap Acceptance model parameters
	static const double MLC_parameters[5];		//Mandatory Lane Changing parameters

	//Parameters represent the location of the vehicle on the road
	//Since the classes of the network haven't be finished, I will use simple type of parameters instead to represent the networks.
	static const link_ testLinks[];
	static const int numOfBadAreas				=	3;
	static const badArea badareas[];				//bad areas, just to check if MLC model can function


	/**********BASIC DATA*************/
private:
	//absolute Movement-related variables
	double timeStep;			//time step size of simulation
	double xPos;
	double yPos;
	double xVel;
	double yVel;
	double xAcc;
	double yAcc;

	//relative Movement-related variables in link
	double xPos_;
	double yPos_;
	double xVel_;
	double yVel_;
	double speed_;
	double acc_;
	double xAcc_;
	double yAcc_;
	double xDirection;			//x direction of the current link
	double yDirection;			//y direction of the current link

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
	double getLength(){return length;}

	//for coordinate transform
	void getFromParent();		///<get current data from parent buffer data
	void setToParent();			///<set next data to parent buffer data
	void abs2relat();			///<transform absolute coordinate to relative coordinate
	void relat2abs();			///<transform relative coordinate to absolute coordinate

	double getMaxAcceleration(){return maxAcceleration;}
	double getNormalDeceleration(){return normalDeceleration;}
	double getMaxDeceleration(){return maxDeceleration;}
	double getDistanceToNormalStop(){return distanceToNormalStop;}
	double getDistance();


	/***********SOMETHING BIG BROTHER CAN RETURN*************/
private:
	Agent* leader;				///<Pointer pointing to leading vehicle
	double leader_xPos_;		//parameters of leading vehicle in relative coordinate
	double leader_yPos_;
	double leader_xVel_;
	double leader_yVel_;
	double leader_xAcc_;
	double leader_yAcc_;
	int currentLink;				//current link ID
	int currentLane;				//current lane ID
	Agent* LF;				//pointer pointing to the vehicle in the left lane and in front of the vehicle in the smallest distance
	Agent* LB;				//left lane, behind, closest
	Agent* RF;				//right lane, front, closest
	Agent* RB;				//right lane, behind, closest

public:
	void updateLeadingDriver();				///<this may be a function of big brother later
	Agent* getLeadingDriver(){return leader;}
	int getLink();							//return the ID of current link
	int getLane();							//return the ID of the lane where the vehicle is on
	Agent* getNextForBDriver(bool isLeft,bool isFront);	///<for updating LF LB RF RB
	bool checkIfOnTheLane(double y);		///<give y position, check if it is a lane's y position


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
	void makeAcceleratingDecision();				///<decide acc
	double breakToTargetSpeed();					///<return the acc to a target speed within a specific distance
	double accOfEmergencyDecelerating();			///<when headway < lower threshold, use this function
	double accOfCarFollowing();						///<when lower threshold < headway < upper threshold, use this function
	double accOfMixOfCFandFF();						///<when upper threshold < headway, use this funcion
	double accOfFreeFlowing();						///<is a part of accofMixOfCFandFF
	double getTargetSpeed(){return targetSpeed;}

	//for lane changing decision
private:
	double VelOfLaneChanging;	//perpendicular with the lane's direction
	int changeMode;				//DLC or MLC
	double changeDecision;		//1 for right, -1 for left, 0 for current
	bool ischanging;			//is the vehicle is changing the lane
	bool isback;				//in DLC: is the vehicle get back to the lane to avoid crash
	bool isWaiting;				//in MLC: is the vehicle waiting acceptable gap to change lane
	int fromLane;				//during lane changing, the lane the vehicle leaves
	int toLane;					//during lane changing, the lane the vehicle approaches
	double satisfiedDistance;	//the smallest space ahead which driver is satisfied with so that he/she won't change lane
	double dis2stop;			//distance to where critical location where lane changing has to be made
								//currently, we made it infinite
	double avoidBadAreaDistance;		//when vehicle want to change lane, if the vehicle change to the lane
										//    and the space between the vehicle and the bad area is smaller than
										//    this distance, the vehicle won't change lane(because later it should change again)
public:
	unsigned int gapAcceptance(int type); 	///<check if the gap of the left lane and the right lane is available
	double lcCriticalGap(int type,		// 0=leading 1=lag + 2=mandatory (mask)
			double dis,					// from critical pos
			double spd,					// spd of the follower
			double dv					// spd difference from the leader));
			);								///<use Kazi LC Gap Model to calculate the critical gap
	int checkIfBadAreaAhead();				///<find the closest bad area ahead which the vehicle may knock on(see details in Driver.cpp)
	int findClosestBadAreaAhead(int lane);	///<find the closest bad area ahead in specific lane
	double makeLaneChangingDecision();					///<Firstly, check if MLC is needed, and then choose specific model to decide.
	double checkIfMandatory();							///<check if MLC is needed, return probability to MLC
	double calcSideLaneUtility(bool isLeft);			///<return utility of adjacent gap
	double makeDiscretionaryLaneChangingDecision();		///<DLC model, vehicles freely decide which lane to move. Returns 1 for Right, -1 for Left, and 0 for neither.
	double makeMandatoryLaneChangingDecision();			///<MLC model, vehicles must change lane, Returns 1 for Right, -1 for Left.
	void excuteLaneChanging();			///<to execute the lane changing, meanwhile, check if crash will happen and avoid it
	bool checkForCrash();				///<to check if the crash may happen
	/*
	 * Now I assume that vehicles must make lane changing when they face "bad area" ahead of them.
	 * The so called bad area is where vehicles can not go.
	 * Now there is no parts specially designed for avoiding bad areas.
	 * The decision making process is merged into discretionary lane changing.
	 * Just add parameters and functions below to search for the bad areas.
	 * */


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
