/*
 * TrainDriverFacets.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */
#pragma once
#include <atomic>
#include "conf/settings/DisableMPI.h"
#include "entities/roles/RoleFacets.hpp"
#include "TrainPathMover.hpp"
#include "TrainDriver.hpp"
namespace sim_mob
{
namespace medium
{
class TrainDriver;

class TrainBehavior : public BehaviorFacet
{
public:
	explicit TrainBehavior();
	virtual ~TrainBehavior();

	/**
	 * Virtual overrides
	 */
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

	TrainDriver* getParentDriver() const;
	void setParentDriver(TrainDriver* parentDriver);

protected:
	/**Pointer to the parent Driver role. */
	TrainDriver* parentDriver;
};

enum TRAINCASE{NORMAL_CASE,STATION_CASE};
class TrainMovement : public MovementFacet
{
public:
	explicit TrainMovement(std::string lineId);
	virtual ~TrainMovement();

	//virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

	TrainDriver* getParentDriver() const;
	static bool areColumnNamesAdded;
	void setParentDriver(TrainDriver* parentDriver);
	/**
	 * get the object of path mover
	 * @return the reference to the object of path mover
	 */
	 const TrainPathMover& getPathMover() const ;
	/**
	 * get the object of platform mover
	 * @return the reference to the object of platform mover
	 */
	Platform* getNextPlatform();
	/**
	 * move train forward
	 * @return true if train successfully move forward.
	 */
	bool moveForward();

	/**
	 * Whether the train already arrive at one platform
	 * @return true if train already move in one platform
	 */
	bool isStopAtPlatform();

	/**
	 * Whether the train already arrive at last platform
	 * @return true if train already arrive at terminal platform
	 */
	bool isAtLastPlaform();

	/**
	 * make train leave from current platform
	 */
	bool leaveFromPlaform();

	/**
	 * assign the train to the first platform
	 */
	void arrivalAtStartPlaform() const;

	/**
	 * inform the train arrival at last platform
	 */
	void arrivalAtEndPlatform() const;

	/**
	 * This function prepares for U turn in case of disruption or other scenarios
	 * Firstly it checks if any trains are approaching the platform where it has to be
	 * teleported after U turn ,if it is clear
	 * Then it checks the safe Distance of train ahead in the opposite line afetr the platform
	 * If everything is positive then it prepares to take U turn
	 */
	void prepareForUTurn();

	/**
	 * get distance to next train
	 * @param next is a pointer to next TrainDriver
	 * @param isSafed indicate whether conside safe distance
	 * @return the distance to next train
	 */
	double getDistanceToNextTrain(const TrainDriver* next, bool isSafed=true) ;

	/**
	 * This function gets the distance to next platform
	 * @param nextDriver is the pointer to the next platform whose distance from current train is to be obtained
	 * @return distance to the next platform
	 */
	double getDistanceToNextPlatform(const TrainDriver* nextDriver);

	/**
	 * This function resets the safe Headway (speed limit of train based on train ahead) as requested by service controller
	 * @param safeHeadWay is the value of safeHeadway to be reseted
	 */
	void resetSafeHeadWay(double safeHeadWay);

	/**
	 * This function resets the safe Distance (Distance of train to be maintained from the train ahead to be safe ) as requested by service controller
	 * @param safeDistance is the value of the safeDistance to be reseted
	 */
	void resetSafeDistance(double safeDistance);

	/**
	 * This interface makes the train take Uturn ,that is the train gets teleported to the opposite platform on the same station and continues
	 * its journey in reverse direction..This can happen in case of disruption when the train is at last platform before disrupted region.
	 * @param StationName is the name of the station
	 */
	void takeUTurn(std::string stationName);

	/**
	 * Safe check of before taking Uturn
	 * To see if the any trains are approaching the opposite platform when uturn is to be taken
	 * If any trains are approaching the opposite platform then wait till the trains get cleared
	 * @param platformName is the name of the platform to check if the trains are approaching there
	 * @lineId is the id of the line in whose platform the approaching trains are to be checked
	 */
	bool checkIfTrainsAreApprochingOrAtPlatform(std::string platformName,std::string lineID) const;

	/**
	 * Checking the safe distance before teleporting to opposite line
	 * Checking safe distance is only enough,safe headway not required
	 * As the train will initially be at rest when it is teleported
	 * @param platformNo is the name of the platform from where the safe distance ahead has to be checked
	 * @lineId is the id of the line of the platform
	 * @return bool whether its safe or not
	 */
	bool checkSafeDistanceAheadBeforeTeleport(std::string platformNo,std::string lineID) const;

	/**
	 * gets the distance from start of line to particular platform
	 * @param lineId is the id of the line from the beginning of which the distance is to be obtained
	 * @param platform the pointer to the platform whose distance is to be obtained
	 * @return the distance from start to the platform specied
	 */
	double getDistanceFromStartToPlatform(std::string lineID,Platform *platform) const;

   /**
    * This function gets the total distance covered by the train
    * @return total distance covered by the train
    */
	double getTotalCoveredDistance();

	/**
	 * This function sets the pointer to next platform for the train
	 * @param platfrom is the pointer to the next platform
	 */
	void  setNextPlatform(Platform *platform);

	/**
	 * Teleports to platform in case of Uturn
	 * The path setting ,polyline blocks ,polypoint is done by function in train path mover
	 * This function invokes those functions in trainpath mover
	 * @param platformName is the name of the platform where the train is to be teleported
	 */
	void teleportToPlatform(std::string platformName);

	/**
	 * This function gets the next next platform by invoking train platform mover
	 * @return the pointer to the next platform
	 */
	Platform *getNextPlatformFromPlatformMover();

	/**
	 * This function produces the movement result for diagnosis file pt_mrt_move.csv
	 */
	void produceMoveInfo();

	/**
	 * This function produces the passenger info file pt_mrt_passengernfo.csv
	 */
	void passengerInfo();

	/**
	 * This function changes the trip of train while teleporting that is the line is set to opposite line
	 * The corresponding platforms in the route are set
	 * Also the platform where the train will be teleported is set
	 */
	void changeTrip();

	/** 
	 * This function updates the platform list ,that is platform list to be ignored
	 * @param isToBeRemoved is the bool  to indicate whether the train is to be removed from current station agent
	 * or not
     */
	bool updatePlatformsList(bool &isToBeRemoved); //can be done by firing an event as well

	/** 
	 * resets from Station case to normal case and vice versa
	 * @param trainCase is the enum to which the moving case has to be resetted to
	 */
	void resetMovingCase(TRAINCASE trainCase);

	/** 
	 * This function sets the train status if it is in disrupted region or if it cannot move ahead to
	 * next platform due to disruption,irrespective of whether it is on platform or not
	 * @param disruptedState is the bool indicatind disruption or not
	 */
	void setDisruptedState(bool disruptedState);

	/** 
	 * This function gets the status of train whether it is in disrupted state or not
	 * @return bool indicated whether its disrupted state or not
	 */
	bool getDisruptedState() const;

	/**
	 * This function checks if the train is stranded between platform during disruption
	 * @return bool if it is stranded between the platform during disruption
	 */
	bool isStrandedBetweenPlatform();

	/**
	 * checks if the current point is the point to be stopped as requested by service controller
	 * Sets the stopping parameters like duration ,point
	 * @return bool if the stop point is present at trains current location
	 */
	bool isStopPointPresent();

	/**
	 * This function gets the safe distance of the train to be  maintained from the train ahead
	 * @return the safe distance required for the train to maintain to next train
	 */
	double getSafeDistance() const;

	/**
	 * checks if the route is set if the train is inserted in unscheduled manner
	 * @return bool indicating whether the route is set or not
	 */
	bool isRouteSet() const;

	/** 
	 * sets the route status which is set so later it need not to set
	 * @param status indicates whether the route is set or not
	 */
	void setRouteStatus(bool status);

	/**
	 * This function sets the flag to ignore the safe distance to next train by service controller
	 * @param ignore is the bool to indicate whether to ignore or uningore if previously ignored
	 */
	void setIgnoreSafeDistanceByServiceController(bool ignore);

	/**
	 * This function sets the flag to ignore the safe headway to next train by service controller
	 * @param ignore is the bool to indicate whether to ignore or uningore if previously ignored
	 */
	void setIgnoreSafeHeadwayByServiceController(bool ignore);

	/**
	 * This function gets the current speed of the train in kmph unit
	 * @return current speed of the train
	 */
	double getCurrentSpeed() const;

	/**
	 * This function gets the safe headway of the train
	 * @return safe headway of the train
	 */
	double getSafeHeadWay() const;

	/**
	 * This function sets the user specified time for uturn by service controller
	 * @param time is the user specified time for uturn
	 */
	void setUserSpecifiedTimeToTakeUturn(double time) ;

	/**
	 * This function gets the platform by offset for the train
	 * @param offset is the offset from current platform
	 */
	std::string getPlatformByOffset(int offset);

	/**
	 * This function sets the move to true or false when updated by the train station agent
	 * @param toMove is the bool whether to move or not
	 */
	void setToMove(bool toMove);

	/**
	 * This function gets the move status of the train ,whether to move or not
	 * @bool move status of the train whether to move or not
	 */
	bool getToMove();

	/**
	 * This function sets the no move timeslice that is the time slice when not to move
	 * @param ts is the no move timeslice
	 */
	void  setNoMoveTimeslice(int ts);

	/**
	 * This function indicates whether the train should stop due to disruption at the current platform if it is serving it
	 * or next platform if it is between platforms
	 * @param aheadDriver is the pointer to next driver of the train
	 * @return indicates whether to stop due to disruption or not
	 */
	bool shouldStopDueToDisruption(TrainDriver *aheadDriver);

	/**
	 * This function indicates whether the train ahead of current train should stop due to disruption at current platform
	 * it is serving or the next platform if it is between platforms
	 * @param aheadDriver is the pointer to next driver of the train
	 * @return indicates whether to stop due to disruption or not
	 */
	std::string shouldTrainAheadStopDueToDisruption(TrainDriver *aheadDriver);

	/**
	 * This function indicates whether the uturn platform for the current train is on the way on its route
	 * @return whether the uturn platform is on the way
	 */
	bool isUTurnPlatformOnTheWay();

	/**
	 * This function gets the next Uturn Platform on the way
	 * @return name of next uturn platform on the way
	 */
	std::string getNextUturnPlatform();

	/**
	 * This function finds the nearest stop point to the train from the current position of the train
	 * @param stopPoints is the vector of stop point entities where the train is supposed to stop
	 * @param distance is the distance to nearest stop point
	 * @param maxDeceleration is the maximum deceleration which can be used for stopping at the stop point
	 * @return the stop point ntity which is nearest to current position of the train
	 */
	std::vector<StopPointEntity>::iterator findNearestStopPoint(std::vector<StopPointEntity> &stopPoints,double &distance,double &maxDeceleration);

	/**
	 * This function sets the flag whether the train should ignore all the platforms on its way thereafter
	 * @param action whether to ignore all the platforms
	 */
	void setShouldIgnoreAllPlatforms(bool action);

	/**
	 * This function gets the train platform mover for the train
	 * @return reference to the trainPlatformMover for the train
	 */
	TrainPlatformMover& getTrainPlatformMover();

protected:
	virtual TravelMetric& startTravelTimeMetric();
	virtual TravelMetric& finalizeTravelTimeMetric();

private:
	/**Pointer to the parent Driver role.*/
	TrainDriver* parentDriver;
	/**Train path mover*/
	TrainPathMover trainPathMover;
	/**Train platform mover*/
	TrainPlatformMover trainPlatformMover;

	TrainPlatformMover trainPlatformMover_accpos;
	/**safe distance*/
	double safeDistance;
	/**safe headway*/
	double safeHeadway;
	/**next platform*/
	Platform* nextPlatform;
	/**the locker for this class*/
	boost::mutex facetMutex;
	/**flag whether to reset moving case or not */
	bool forceResetMovingCase=false;
	/**bool to indicate whether its disrupted state or not */
	bool isDisruptedState=false;
	/**bool to indicate whether the train is stranded between platform during disruption */
	bool isStrandedBetweenPlatforms_DisruptedState=false;
	boost::mutex safeDistanceLock;
	boost::mutex safeHeadwayLock;
	/**bool to indicate whether the route is set or not */
	bool routeSet=false;
	/**bool to indicate whether the train is unscheduled or not */
	bool isUnscheduled=false;
	/**bool to indicate whether to ignore safe distance  as requested by service controller */
	bool ignoreSafeDistance_RequestServiceController=false;
	/**bool to indicate whether to ignore safe headway  as requested by service controller */
	bool ignoreSafeHeadway_RequestServiceController=false;
	/**enum to indicate the force reseted case ,stopping case or normal case */
	TRAINCASE forceResetedCase;
	/**bool to indicate whether to invoke the movement frame tick of the train or not */
	bool toMove=true;
	/**bool to indicate the timeslice when not to invoke the frame tick of the train */
	int noMoveTimeSlice;
	/** indicates the time remaining for uturn of the train */
	double waitingTimeRemainingBeforeUturn=0;
	/**time specified by user to uturn */
	double userSpecifiedUturnTime=0;
	/**bool to indicate whether the train is waiting for uturn or not */
	bool isWaitingForUturn=false;
	/**bool to indicate whether to ignore all the platforms of the train */
	bool shouldIgnoreAllPlatforms=false;
	/**the travel time between the stations of the train not including dwell time*/
	bool calculatedTravelTime = false;
	/** bool which indicates whether the subsequent frame tick call is the first frame tick or not */
	bool isFirstMove = true;
	/** the time of departure of the train from the previous platform*/
	uint32_t startTimeOfNextStationStretch=0;


private:
	/**
	 * get current speed limit
	 * @return current speed limit
	 */
	double getRealSpeedLimit();
	/**
	 * get effective speed
	 * @return effective speed
	 */
	double getEffectiveAccelerate();
	/**
	 * get effective moving distance
	 * @return effective distance
	 */
	double getEffectiveMovingDistance();
	/**
	 * is station case happen?
	 * @return true when station case happen
	 */
	bool isStationCase(double disToTrain, double disToPlatform,double disToStopPoint, double& effectDis);

	/**
	 * This function erases the stop point for the train
	 * @param pointItr is the itr to the vector of stop point
	 */
	void eraseStopPoint(std::vector<PolyPoint>::iterator pointItr);

	/**
	 * This function reassigns the train to other station agents
	 * @param prevPlatform is the platform of the station of current train station agent
	 * @param newPlatform is the platform of the station of the new train station agent
	 * @param isToBeRemoved is the bool reference indicating whether the train is to be removed from current train staion agent
	 */
	void reAssignTrainsToStationAgent(Platform *prevPlatfrom,Platform *newPlatform,bool &isToBeRemoved);

	/**
	 * This function produces the dwell time info for the train
	 */
	void produceDwellTimeInfo();
};


}

} /* namespace sim_mob */


