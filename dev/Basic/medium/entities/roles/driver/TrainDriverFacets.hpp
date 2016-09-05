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
namespace sim_mob {
//class TrainPathMover;
namespace medium{
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
	explicit TrainMovement();
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

	/*
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


	/* get distance to next platform*/
	double getDistanceToNextPlatform(const TrainDriver* nextDriver);

	/* resets the safe Headway (speed limit of train based on train ahead) as requested by service controller */
	void resetSafeHeadWay(double safeHeadWay);

	/* resets the safe Distance (Distance of train to be maintained from the train ahead to be safe ) as requested by service controller */
	void resetSafeDistance(double safeDistance);
	/*makes the train take Uturn ,that is the train gets teleported to the opposite platform on the same station and continues
	 * its journey in reverse direction..This can happen in case of disruption when the train is at last platform before disrupted region.
	 */
	void TakeUTurn(std::string stationName);

	/* Safe check of before taking Uturn
	 * To see if the any trains are approaching the opposite platform when uturn is to be taken
	 * If any trains are approaching the opposite platform then wait till the trains get cleared
	 *
	 */

	bool checkIfTrainsAreApprochingOrAtPlatform(std::string pltaformName,std::string lineID);


	/*
	 * Checking the safe distance before teleporting to opposite line
	 * Checking safe distance is only enough,safe headway not required
	 * As the train will initially be at rest when it is teleported
	 */

	bool checkSafeHeadWayBeforeTeleport(std::string platformNo,std::string lineID);

	/* gets the distance from start of line to particular platform */
	double getDistanceFromStartToPlatform(std::string lineID,Platform *platform);
   /* GETS TOTAL DISTANCE COVERED BY THE TRAIN */
	double getTotalCoveredDistance();

	/* sets the pointer to next platform */
	void  setNextPlatform(Platform *platform);

	/*
	 * Teleports to platform in case of Uturn
	 * The path setting ,polyline blocks ,polypoint is done by function in train path mover
	 * This function invokes those functions in trainpath mover
	 */
	void teleportToPlatform(std::string platformName);

	/* gets the next next platform by invoking train platform mover
	 *
	 */
	Platform *getNextPlatformFromPlatformMover();
	/**
	 * produce movement result for diagnosis
	 */

	/* produces the file pt_mrt_move which has the information of train movement*/
	void produceMoveInfo();

	void passengerInfo();
	/*
	 * changes the trip of train while teleporting that is the line is set to opposite line
	 * The corressponding platforms in the route are set
	 * Also the platform where the train will be teleported is set
	 */
	void changeTrip();

	/* updates the platform list ,that is platform list to be ignored */
	bool updatePlatformsList(bool &isToBeRemoved); //can be done by firing an event as well

	/* resets from Station case to norma case and vice versa,yet to be implemented */
	void resetMovingCase(TRAINCASE trainCase);

	/* sets the train status if it is in disrupted region irrespective of whether it is on platform or not */
	void setDisruptedState(bool disruptedState);

	/* gets the status of train whether it is in disrupted state or not */
	bool getDisruptedState();

	/* checks if the train is stranded betwwen platform during disruption*/
	bool isStrandedBetweenPlatform();

	/* checks if the current point is the point to be stopped as requested by service controller
		 * Sets the stopping parameters like duration ,point
		 * */
	bool isStopPointPresent();

	/* gets the safe distance of the train to be  maintained from the train ahead */
	double getSafeDistance();

	/* checks if the route is set if the train is inserted in unscheduled manner */
	bool isRouteSet();
	/* sets the route status which is set so later it need not to set */
	void setRouteStatus(bool status);

	void setIgnoreSafeDistanceByServiceController(bool ignore);
	void setIgnoreSafeHeadwayByServiceController(bool ignore);

	std::string getPlatformByOffset(int offset);
	bool isUturnDueToDisruption();
	void setToMove(bool toMove);
	bool getToMove();
	void  setNoMoveTimeslice(int ts);

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
	bool forceResetMovingCase=false;
	bool isDisruptedState=false;
	bool uTurnDueToDisruption=false;
	bool isStrandedBetweenPlatforms_DisruptedState=false;
	boost::mutex safeDistanceLock;
	boost::mutex safeHeadwayLock;
	bool routeSet=false;
	bool isUnscheduled=false;
	bool ignoreSafeDistance_RequestServiceController=false;
	bool ignoreSafeHeadway_RequestServiceController=false;
	TRAINCASE forceResetedCase;
	bool toMove=true;
	int noMoveTimeSlice;

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



	void produceDwellTimeInfo();
};


}

} /* namespace sim_mob */


