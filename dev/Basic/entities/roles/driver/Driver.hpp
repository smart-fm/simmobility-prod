/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <limits>
#include <vector>
#include <math.h>
#include <set>
#include "../Role.hpp"
#include "../pedestrian/Pedestrian.hpp"
#include "../../Person.hpp"
#include "../../Signal.hpp"
#include "../../AuraManager.hpp"
#include "../../../buffering/Buffered.hpp"
#include "../../../buffering/BufferedDataManager.hpp"
#include "../../../geospatial/Link.hpp"
#include "../../../geospatial/RoadSegment.hpp"
#include "../../../geospatial/Lane.hpp"
#include "../../../geospatial/Node.hpp"
#include "../../../geospatial/UniNode.hpp"
#include "../../../geospatial/MultiNode.hpp"
#include "../../../geospatial/LaneConnector.hpp"
#include "../../../geospatial/StreetDirectory.hpp"
#include "../../../geospatial/Crossing.hpp"
#include "../../../perception/FixedDelayed.hpp"


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
	static const double MAX_ACCELERATION		=	+10.0;//10m/s*s
	static const double MAX_DECELERATION		=	-10.0;

	//Something I have to define

	static const double FLT_EPSILON				=	0.01;	//the smallest double
	static const double MAX_NUM;							//regard as infinity
	static const int MAX_NUM_INT;
	static const double hBufferUpper			=	1.6;	//upper threshold of headway
	static const double hBufferLower			=	0.8;	//lower threshold of headway

	//parameters(refer to MITSIMLab data files)
	static const double CF_parameters[2][6];	//Car Following parameters
	static const double GA_parameters[4][9];	//Gap Acceptance model parameters
	static const double MLC_parameters[5];		//Mandatory Lane Changing parameters




	/**********BASIC DATA*************/
private:
        //Sample stored data which takes reaction time into account.
	const static size_t reactTime = 1500; //1.5 seconds
	FixedDelayed<Point2D*> perceivedVelocity;
	FixedDelayed<Point2D*> perceivedVelocityOfFwdCar;
	FixedDelayed<centimeter_t> perceivedDistToFwdCar;
	//absolute Movement-related variables
	double timeStep;			//time step size of simulation
	int xPos;
	int yPos;
	double xVel;
	double yVel;
	double xAcc;
	double yAcc;

	//absolute position of the target start point on the next link
	//used for intersection driving behavior
	int xPos_nextLink;
	int yPos_nextLink;


	//relative Movement-related variables in link
	int xPos_;
	int yPos_;
	int xPosCrossing_; //relative x coordinate for crossing, the intersection point of crossing's front line and current polyline
	double xVel_;
	double yVel_;
	double speed;
	double speed_;
	double acc_;
	double xAcc_;
	double yAcc_;
	double xDirection;			//x direction of the current polyline segment
	double yDirection;			//y direction of the current polyline segment
	double crossingFarX;
	double crossingFarY;
	double crossingNearX;
	double crossingNearY;


	Point2D origin;
	Point2D goal;
	const Node* destNode;				//first, assume that each vehicle moves towards a goal
	const Node* originNode;				//when a vehicle reaches its goal, it will return to origin and moves to the goal again
	bool isGoalSet;				//to check if the goal has been set
	bool isOriginSet;			//to check if the origin has been set
	double length;				//length of the vehicle
	double width;				//width of the vehicle

	double maxAcceleration;
	double normalDeceleration;
	double maxDeceleration;
	double distanceToNormalStop;
	double maxLaneSpeed;

public:
	int getTimeStep() const {return timeStep;}
	double getLength() const {return length;}

	//for coordinate transform
	void getFromParent();		///<get current data from parent buffer data
	void setToParent();			///<set next data to parent buffer data
	void abs_relat();           ///<compute transformation vectors
	void abs2relat();			///<transform absolute coordinate to relative coordinate
	void relat2abs();			///<transform relative coordinate to absolute coordinate
	double feet2Unit(double feet);
	double unit2Feet(double unit);

	double getMaxAcceleration() const {return maxAcceleration;}
	double getNormalDeceleration() const {return normalDeceleration;}
	double getMaxDeceleration() const {return maxDeceleration;}
	double getDistanceToNormalStop() const {return distanceToNormalStop;}
	void output(frame_t frameNumber);

	/****************IN REAL NETWORK****************/
private:
	std::vector<const Link*> linkPath;
	const Link* currLink_;
	const Link* nextLink_;
	const RoadSegment* currRoadSegment_;
	const Lane* currLane_;
	const Lane* nextLane_;
	const Lane* leftLane_;
	const Lane* rightLane_;
	const Node* currNode_;
	const Link* desLink_;
	double currLaneOffset_;
    double currLinkOffset_;
	double traveledDis_; //the distance traveled within current time step
	size_t linkIndex;
	size_t RSIndex;
	size_t startIndex;
	size_t endIndex;
	size_t desStartIndex;
	size_t desEndIndex;
	size_t currLaneID_;
	size_t currLaneIndex_;
	size_t polylineIndex_;
	std::vector<const Lane*> targetLane;
	StreetDirectory::LaneAndIndexPair laneAndIndexPair;
	const std::vector<sim_mob::Point2D>* currLanePolyLine;
	const std::vector<sim_mob::Point2D>* desLanePolyLine;
	const std::vector<sim_mob::RoadSegment*>* roadSegments;
	Point2D currPolyLineSegStart;
	Point2D currPolyLineSegEnd;
	Point2D desPolyLineStart;
	Point2D desPolyLineEnd;
	Point2D entryPoint; //entry point for crossing intersection
	int xTurningStart;
	int yTurningStart;
	int polyLineSegLength;
	double currLaneLength;
	int disToCrossing; //in the range of this distance(5m), the vehicle can aware of a crossing in front.
	double xDirection_entryPoint;
	double yDirection_entryPoint;
	int disToEntryPoint;
	bool isCrossingAhead;
	bool closeToCrossing;
	bool isForward;
	bool isReachGoal;
	bool lrEnterNewLane;
	std::vector<const Agent*> nearby_agents;
	int distanceInFront;
	int distanceBehind;

public:
	Buffered<Link*> currLink;
	Buffered<RoadSegment*> currRoadSegment;
	Buffered<Lane*> currLane;
	Buffered<size_t> polylineIndex;
	Buffered<double> offsetInPolyline;

public:
	const Link* getCurrLink(){return currLink_;}
	const RoadSegment* getCurrRoadSegment(){return currRoadSegment_;}
	const Lane* getCurrLane(){return currLane_;}
	size_t getPoylineIndex(){return polylineIndex_;}
	double getCurrLaneOffset(){return currLaneOffset_;}
	double getCurrLinkOffset(){return currLinkOffset_;}
	double getCurrLaneLength(){return currLaneLength;}
	double getRelatXvel() const{return xVel_;}
	double getRelatXacc() const{return xAcc_;}
	size_t getLane() const {return currLaneID_;}


private:
	void getTargetLane();
	bool isReachPolyLineSegEnd();
	bool isReachLastPolyLineSeg();
	bool isReachLastRS();
	void updateCurrInfo(unsigned int mode);
	void updateAdjacentLanes();
	void updateRSInCurrLink();
	void findCrossing();
	bool isCloseToCrossing();
	bool isReachLinkEnd();
	bool isLeaveIntersection();
	void chooseNextLaneIntersection();
	int disToObstacle(unsigned obstacle_offset);
	const Link* findLink(const MultiNode* start, const MultiNode* end);
	void setBackToOrigin();
	void updateNearbyAgents();
	void updateCurrLaneLength();
	void updateDisToLaneEnd();
	bool isGoalReached();
	void setGoal();
	void setOrigin();
	void updateAcceleration();
	void updateVelocity();
	void updatePosition();
	void updatePolyLineSeg();
	void lcPosUpdate();
	void updateStartEndIndex();
	size_t getLaneIndex(const Lane* l,const RoadSegment* r);


	/***********SOMETHING BIG BROTHER CAN RETURN*************/
private:
	Agent* leader;				///<Pointer pointing to leading vehicle

	const Driver* CFD;
	const Driver* CBD;
	const Driver* LFD;
	const Driver* LBD;
	const Driver* RFD;
	const Driver* RBD;
	const Pedestrian* CFP;
	const Pedestrian* LFP;
	const Pedestrian* RFP;


	/***********FOR DRIVING BEHAVIOR MODEL**************/
	//parameters
private:
	double targetSpeed;			//the speed which the vehicle is going to achieve
	double minCFDistance;				//the distance between subject vehicle to leading vehicle
	double minCBDistance;
	double minLFDistance;
	double minLBDistance;
	double minRFDistance;
	double minRBDistance;
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
	double getTargetSpeed() const {return targetSpeed;}

	//for lane changing decision
private:
	double VelOfLaneChanging;	//perpendicular with the lane's direction
	int changeMode;				//DLC or MLC
	int changeDecision;		//1 for right, -1 for left, 0 for current
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




	/**************BEHAVIOR WHEN APPROACHING A INTERSECTION***************/
public:
	bool reachSignalDecision();
	void updateAngle();
	void IntersectionVelocityUpdate();
	void modifyPosition();
	bool isReachSignal();
	void IntersectionDirectionUpdate();
	void UpdateNextLinkLane();
	void enterNextLink();
	bool isInTheIntersection();
	bool isReachCrosswalk();
	void updateTrafficSignal();

private:
	Signal* trafficSignal;
	double angle;
	bool inIntersection;

	/**************COOPERATION WITH PEDESTRIAN***************/
public:
	bool isPedestrianAhead();

};



}
