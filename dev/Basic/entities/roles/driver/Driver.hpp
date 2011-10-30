/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <limits>
#include <vector>
#include <math.h>
#include <set>

#include "entities/roles/Role.hpp"
#include "buffering/Buffered.hpp"
#include "geospatial/StreetDirectory.hpp"
#include "perception/FixedDelayed.hpp"
#include "entities/Signal.hpp"
#include "entities/vehicle/Vehicle.hpp"


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

//Forward declarations
class Pedestrian;
class Signal;
class Link;
class RoadSegment;
class Lane;
class Node;
class MultiNode;
struct DPoint;


class Driver : public sim_mob::Role {
public:
	Driver (Agent* parent);			//to initiate
	virtual void update(frame_t frameNumber);
	void assignVehicle(Vehicle* v) {vehicle = v;}

private:
	static const double MAX_ACCELERATION		=	+5.0;//10m/s*s
	static const double MAX_DECELERATION		=	-5.0;

	//Something I have to define

	static const double FLT_EPSILON				=	0.01;	//the smallest double
	static const double MAX_NUM;							//regard as infinity
	static const double hBufferUpper			=	1.6;	//upper threshold of headway
	static const double hBufferLower			=	0.8;	//lower threshold of headway

	//parameters(refer to MITSIMLab data files)
	static const double CF_parameters[2][6];	//Car Following parameters
	static const double GA_parameters[4][9];	//Gap Acceptance model parameters
	static const double MLC_parameters[5];		//Mandatory Lane Changing parameters




	/**********BASIC DATA*************/
private:

	Vehicle* vehicle;
        //Sample stored data which takes reaction time into account.
	const static size_t reactTime = 1500; //1.5 seconds
	FixedDelayed<DPoint*> perceivedVelocity;
	FixedDelayed<DPoint*> perceivedVelocityOfFwdCar;
	FixedDelayed<centimeter_t> perceivedDistToFwdCar;
	//absolute Movement-related variables
	double timeStep;			//time step size of simulation
	double speed;

	double perceivedXVelocity;
	double perceivedYVelocity;
	double perceivedXVelocity_;
	double perceivedYVelocity_;

	//absolute position of the target start point on the next link
	//used for intersection driving behavior
	int xPos_nextLink;
	int yPos_nextLink;

	int xPosCrossing_; //relative x coordinate for crossing, the intersection point of crossing's front line and current polyline
	double speed_;
	double acc_;
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

	double maxAcceleration;
	double normalDeceleration;
	double maxDeceleration;
	double distanceToNormalStop;
	double maxLaneSpeed;

public:
	int getTimeStep() const {return timeStep;}

	//for coordinate transform
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
	const Link* currLink;
	const Link* nextLink;
	const RoadSegment* currRoadSegment;
	const Lane* currLane;
	const Lane* nextLaneInNextLink;
	const Lane* leftLane;
	const Lane* rightLane;
	const Node* currNode;
	const Link* desLink;
	double currLaneOffset;
    double currLinkOffset;
	double traveledDis; //the distance traveled within current time step
	size_t linkIndex;
	size_t RSIndex;
	size_t polylineSegIndex;
	size_t currLaneIndex;
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
	double xDirection_entryPoint;
	double yDirection_entryPoint;
	int disToEntryPoint;
	bool isCrossingAhead;
	bool closeToCrossing;
	bool isForward;
	bool isReachGoal;
	bool lcEnterNewLane;
	bool isTrafficLightStop;
	std::vector<const Agent*> nearby_agents;
	int distanceInFront;
	int distanceBehind;

public:
	Buffered<Link*> currLink_;
	Buffered<RoadSegment*> currRoadSegment_;
	Buffered<Lane*> currLane_;
	Buffered<size_t> polylineIndex_;
	Buffered<double> offsetInPolyline_;

public:
	const Link* getCurrLink() const {return currLink;}
	const RoadSegment* getCurrRoadSegment() const {return currRoadSegment;}
	const Lane* getCurrLane() const {return currLane;}
	double getCurrLaneOffset() const {return currLaneOffset;}
	double getCurrLinkOffset() const {return currLinkOffset;}
	double getCurrLaneLength() const {return currLaneLength;}
	const Vehicle* getVehicle() const {return vehicle;}

private:
	bool isReachPolyLineSegEnd();
	bool isReachLastPolyLineSeg();
	bool isReachLastRS();
	bool isCloseToCrossing();
	bool isReachLinkEnd();
	bool isLeaveIntersection();
	bool isGoalReached();
	bool isCloseToLinkEnd();
	void chooseNextLaneIntersection();
	int disToObstacle(unsigned obstacle_offset);
	const Link* findLink(const MultiNode* start, const MultiNode* end);

	void setOrigin();

	void updateCurrInfo(unsigned int mode);
	void updateAdjacentLanes();
	void updateRSInCurrLink();
	void updateAcceleration();
	void updateVelocity();
	void updatePositionOnLink();
	void updatePolyLineSeg();
	void setBackToOrigin();
	void updateNearbyAgents();
	void updateCurrLaneLength();
	void updateDisToLaneEnd();
	void updatePosLC();
	void updateStartEndIndex();
	void updateTrafficSignal();

	void trafficSignalDriving();
	void intersectionDriving();
	void pedestrianAheadDriving();
	void linkDriving();

	void makeLinkPath();
	void findCrossing();

	//helper function, to find the lane index in current road segment
	size_t getLaneIndex(const Lane* l);



	/***********SOMETHING BIG BROTHER CAN RETURN*************/
private:

	//Vehicle* vehicle;
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
	double minPedestrianDis;
	double tsStopDistance;     // distance to stop line
	double space;
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
	bool isLaneChanging;			//is the vehicle is changing the lane
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
	void updateAngle();
	void intersectionVelocityUpdate();
	void modifyPosition();
	bool isReachSignal();
	void IntersectionDirectionUpdate();
	void UpdateNextLinkLane();
	void enterNextLink();
	bool isReachCrosswalk();
	bool isInIntersection() const {return inIntersection;}

private:
	const Signal* trafficSignal;
	double angle;
	bool inIntersection;

	/**************COOPERATION WITH PEDESTRIAN***************/
public:
	bool isPedestrianAhead;

};



}
