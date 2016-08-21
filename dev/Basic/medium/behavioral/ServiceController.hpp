//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include "entities/misc/PublicTransit.hpp"
#include "lua/LuaModel.hpp"
#include "entities/roles/Role.hpp"
#include "entities/Person_MT.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"
#include "entities/roles/driver/TrainDriverFacets.hpp"


namespace sim_mob
{

namespace medium
{

struct TrainIdLineId
{
	int trainId;
	std::string lineId;
};

/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 * TrainServiceController class is an interface for lua apis.The lua functions
 * calls invokes the functions in this class
 * Author: Jabir <jabir@smart.mit.edu>
 */

class ServiceController:  public lua::LuaModel
{
	public:
	static ServiceController* pInstance;
	explicit ServiceController();
	virtual ~ServiceController();
	/* This maps the function names to lua Apis*/
	void mapClasses();
	/* starting function to invoke to use service controller */
	void useServiceController(std::string time);

	/*interface function reset the speed limit of a stretch of blocks
	 *@param speedLimit is the speedlimit in the blocks to be set
	 *@param startStation is the start station of the stretch from where blocks are to be reset
	 *@param endStation is the end station of the stretch till where the blocks are to be reset
	 *@param startTime is time at which the speedLimit should reset
	 *@param endTime is time at which speed limit should to set back to default
	 */
	void resetSpeedLimit(double speedLimit,std::string startStation,std::string endStation,std::string lineId,std::string startTime,std::string endTime);

	/*interface function reset the acceleration limit of the line
	 *@param accelerate is the acceleration limit to be set for the line
	 *@param lineId is the lineid of the line to reset acceleartion limit
	 */
	void resetAcceleration(double accelerate,std::string lineId);

	/*interface to  reset the safe headway of a particular train
	 *@param sec is the safe headway sec to be set
	 *@param trainId is the train id of the train whose headway sec is to be reset
	 *@param lineId is the id of the line where the train with given train id whose headway sec is the be reset
	 */
	void resetSafeHeadwaySec(double sec,int trainId,std::string lineId);

	/*interface to  reset the safe distance of a particular train
	 *@param distance is the safe distance to be set
	 *@param trainId is the train id of the train whose safe distance is to be reset
	 *@param lineId is the id of the line where the train with given train id whose safe distance is the be reset
	 */
	void resetSafeOperationDistance(double distance,int trainId,std::string lineId);

	/*interface to reset operation of train from station case to normal case and from normal case to station case
	 *@param trainId is the train id of the train whose moving case is to be reset
	 *@param lineId is the id of the line where the train with given train id whose moving case is the be reset
	 *@param caseVal is the value of reset ,it can be Station case or Normal case
	 */
	void resetMovingCase(int trainId,std::string lineId,TRAINCASE caseVal);

	/*Interface to set flag to force release the passengers from a given train
	 *@param trainId is the train id of the train whose passengers are to be force released
	 *@param lineId is the id of the line where the train with given train id whose passengers are to be force released
	 *@action is a bool which sets the action to force release or unsets it.
	 */
	void forceReleasePassenegers(int trainId,std::string lineId,bool action);

	/*Interface to override the default waiting time of a train at station if the holding time is more than calculated waiting time
	 *@param platformName is the name of the platform where we need to reset the holding time
	 *@param duration is the duration of reset
	 *@param trainId is the train id of the train whose holding time is to be reset
	 *@param lineId is the id of the line where the train with given train id's holding time is to be reset
	 */
	void resetHoldingTimeAtStation(std::string platformName,double duration,int trainId,std::string lineId);

	/*Interface to terminate the service of all trains currently running on the line specified and stops the future schedule dispatch of trains
	 *@param lineId is the id of the line where train service is to be terminated
	 */
	void terminateTrainService(std::string lineId);

	/*interface to get the waiting time remaining of the train if the train is at the station specified
	 *@param trainId is the id of the train whose waiting time is needed
	 *@param lineId is the id of the line where the train with train id is specified
	 *@param station is the name of station where the dwell time is needed
	 */
	double getDwellTime(int trainId,std::string lineId,std::string station) const;

	/*Interface to get the instance of service controller
	 *@return static instance of service controller
	 */
	static ServiceController* getInstance();

	/*Interface to insert train driver into map for service controller
	 *@param trainId is the id of the train
	 *@param lineId is the id of the line
	 **/
	void insertTrainIdAndTrainDriverInMap(int trainId,std::string lineId,TrainDriver *trainDriver);

	/*Interface to remove the train from the map of service controller
	 *@param trainId is the id of the train
	 *@param line Id is the id of the line of interest
	 *@param trainDriver is the pointer to train driver to be inserted
	 **/
	void removeTrainIdAndTrainDriverInMap(int trainId,std::string lineId,TrainDriver *trainDriver);

	/*Inteface to get the id of the train from the trainDriver passed
	 *@param trainDriver is the pointer to train driver
	 *@return is the id of the train driver passed
	 **/
	int getTrainId(TrainDriver *trainDriver);

	/*Interface to get the lineId of the train from the train driver passed
	 *@param trainDriver is the pointer to the train driver
	 *@return lineId of the train driver*/
	std::string getLineId(TrainDriver *trainDriver);

	/* interface to get  line id of the opposite line eg for NE_1 it gives NE_2 and vice versa
	 *@param lineId is the id of the line whose opposite line id is needed
	 */
	std::string getOppositeLineId(std::string lineId);

	/*Interface to get nearest coming platform for a train
	 *@param trainId is the id of the train whose nearest coming platform is needed
	 *@param lineId is the id of the line where the train id whose nearest coming platform is needed
	 */
	std::string getNextPlatform(int trainId,std::string lineID);

	/*Interface to return the distance to coming platform for the train specified
	 *@param trainId is the id of the train whose distance to nearest platform is needed
	 *@param line id is the id of the line where the train id of interest is to be searched
	 */
	double getDistanceToNextPlatform(std::string lineId,int trainId);

	/*Interface to push the train into inactive pool from inactive pool
	 *@param trainId is the id of the train which has to be pushed into active pool
	 *@param lineId is the id of the line where the train with given tarin id is to be pushed to inactive pool
	 */
	void pushTrainIntoInActivePool(int trainID,std::string lineID);

	/*Interface to  pull out the train from inactive pool and send to active pool.It follows first come first serve
	 *@param lineId is the id of the where a train need to be pulled out from inactive pool
	 */
	void pullOutTrainFromInActivePool(std::string lineID);

	/*Interface to terminate the service for the train after it reaches the nearest platform
	 *@param trainId is the id of the train whose service is to be terminated
	 *@param lineId is the id of the line where the tarin id of interest is to be searched
	 */
	void terminateTrainServiceForTrain(int trainId,std::string lineId);

	/*Interface to get all active trains in line to be dispatched or running
	 *@param lineId is the id of the line
	 *@return returns the vector of trains
	 **/
	std::vector<TrainDriver*> getActiveTrainsInLine(std::string lineId);

	/*Interface to get all the ids of all active trains in line running or waiting to be dispatched
	 *@param lineId is the id of the line
	 *@return is the vector of ids
	 */
	std::vector<int>  getActiveTrainIds(std::string lineId);

	/*Interface to get the number of active trains in the line
	 *@param lineId is the id of the line where we need the number of active trains
	 *@return total number of active trains*
	 */
	int getActiveTrainIdsSize(const std::string lineId) const;

	/*Interface to get the id of the train by index in the list
	 *@param index is the index of the train whose train id is to be fetched
	 *@param lineId is the id of the line where the train of given id is of interest
	 *@return train id
	 */
	int getTrainIdByIndex(int index,std::string lineId) const;

	/*This is the direct Api to perform disruption
	 *@param first station to be affected by disruption
	 *@param last station to be affeted by disruption
	 *@param duration for which the disruption will affect
	 */
	void performDisruption(std::string startStation,std::string endStation,std::string time);

	/**
	 *Interface to inserts stop point for a train on its path at a distance specified
	 *@param trainId is the id of the train which is to be stopped on the stop point specified
	 *@param lineId is the id of the line where the train of interest is to be searched for
	 *@param distance is the distance from the start of the path of given train where the stop point is to be inserted
	 *@param duration is the duration required for the train specified to stop
	 */
	void insertStopPoint(int trainId,std::string lineId,double distance,double duration);

	/**
	 *Interface to update the platform list,basically currently it just specifies the platforms to be ignored as there is only one line
	 *and by default train stops at all platforms
	 *@param trainId is the id of the train whose platform list is to be updated
	 *@param lineId is the id of the line where the train with given id is to be looked for
	 *@param platformsToBeIgnored is the list of platforms to be ignored on th etrains route
	 */
	void updatePlatformList(int trainId,luabridge::LuaRef platformsToBeIgnored,std::string lineId);

	void insertPlatformHoldEntities(std::string platformName,double duration,int trainId,std::string lineId);

	/**
	 *Interface to restrict the passenger movement type.ie boarding,alighting or both
	 *@param platformName is the name of the platform where passenger movement is to be restricted
	 *@param trainId is the id of the train where passenger movement is to be restricted
	 *@param lineId is the id of the line where the train with given ig is to be looked for
	 *@param type is the restriction type ,it can be boarding ,alighting or both
	 */
	void restrictPassengers(std::string platformName,int trainId,std::string lineId,int type);

	/**
	 *Interface to insert an unscheduled train on the line,the starting point can be at any platform
	 *@param lineId is the id of the line where the unscheduled train is to be inserted
	 *@param startTime is the the time when the train will be inserted
	 *@param startStation is the station where the unscheduled train is supposed to start
	 */
	void insertUnScheduledTrain(std::string lineId,std::string startTime,std::string startStation);

	/**
	 *Interface that sets the disruption on the platform
	 *@param startStation is the start station from where the disruption starts
	 *@param endStation is the end station where the disruption ends
	 *@param lineId is the id of the line where the disruption is to be applied
	 */
	void setDisruptedPlatforms(std::string startStation,std::string endStation,std::string lineID);

	/**
	 *Interface that gives the platform by offset that is the platform the train will arrive after skipping the
	 *number of platforms specified ,offset 0 means the coming platform
	 *@param trainId is the id of the train whose platform by offset is needed
	 *@param lineId is the id of the line where the trainId is to be looked for
	 */
	std::string getPlatformByOffset(int trainId,std::string lineId,int offset) const;

	/**
	 *Interface that gives the index of disrupted platform by index specified
	 *@param lineId is the id of the line from where the disrupted platform is needed
	 *@param index is the index of disrupted platform in the list
	 *@return the platformName at that index
	 */
	std::string getDisruptedPlatformByIndex(std::string lineID,int index) const;

	/**
	 *Interface that returns the number of disrupted platforms on a given line
	 *@param lineId is the id of the line of interest
	 *@return total number of disrupted platforms
	 */
	int getDisruptedPlatformsSize(std::string lineID) const;

	/**
	 *Interface to set unset the Uturn signal to the train
	 *@param trainId is the id of the train whose uturn flag is to be set
	 *@param lineId is the id of the line where the train with given id is to be looked for
	 *@param takeUturn is the bool action whether to set or unset the Uturn signal
	 */
	void setUturnFlag(int trainId,std::string lineId,bool takeUturn);

	/*Interface that gives the trainId of the train ahead of the train specified
	 *@param trainId is the id of the train whose next train's id is to be fetched
	 *@param lineId is the id of the line where the train with train id specified to be looked for
	 *@return the if of the train ahead
	 */
	int getTrainIdOfTrainAhead(int trainId,std::string lineId) const;

	/**
	 *Interface the returns future next requested for the train
	 *@param trainId is the id of the train whose next requested is to be fetched
	 *@param lineId is the id of the line where the train of interest is to be specified
	 *@return the next requested for the train
	 **/
	int getNextRequestedMovementActionForTrain(int trainId,std::string lineId) const;

	/**
	 *Interface that sets or clears the ignore safe distance flag for the train
	 *@param trainId is the id of the train whose ignore safe distance flag is to be set
	 *@param lineId is the id of the line of interest
	 *@param ignore is the action to set or unset the ignore flag
	 */
	void setIgnoreSafeDistance(int trainId,std::string lineId,bool ignore);

	/**
	 *Interface that sets or clears the ignore safe headway flag for the train
	 *@param trainId is the id of the train whose ignore safe headway flag is to be set
	 *@param lineId is the id of the line of interest
	 *@param ignore is the action to set or unset the ignore flag
	 */
	void setIgnoreSafeHeadway(int trainId,std::string lineId,bool ignore);

	/**
	 *Interface that clears disruption on the platforms
	 *@param lineId is the id of the line where disruption is to be cleared
	 */
	void clearDisruption(std::string lineId);

	/**
	 *Interface gets the force alight status that is passengers have force alighted or not
	 *@param trainId is the id of the train whose force alight status is to be fetched
	 *@param lineId is the line of interest for that trainId
	 */
	bool getForceAlightStatus(int trainID,std::string lineId) const;

	/**
	 *Interface to set unset force alight status,that is whether passengers have force alighted or not
	 *@param trainId is the id of the train whose force alight status is to be set
	 *@param lineId is the id of line of interest
	 */
	void setForceAlightStatus(int trainId,std::string lineId,bool status);

	/**
	 *Interface returns whether the train is in disrupted region or not
	 *@param trainId is the id of train whose disrupted state is needed
	 *@param lineId is the id of the line where the train of given train id is to be searched
	 *@returns the disrupted state,that is whether the train is in disrupted region or not
	 */
	bool getDisruptedState(int trainId,std::string lineId) const;

	/**
	 *Interface returns if the train is stranded between platform in disrupted region or not
	 *@param trainId is the id of the train of interest
	 *@param lineId is the id of the line where the train with given id is to be searched
	 *@return bool whether the train is stranded between the platform during disruption
	 */
	bool isStrandedDuringDisruption(int trainId,std::string lineId) const;

	/**
	 *Interface that sets the subsequent next requested for the train
	 *@param trainId is the id of the train whose subsequent next requested is to be fetched
	 *@param lineId is the id of the line where the train with given train id has to be searched
	 *@param nextReq is the subsequent next requested(next requested to set set some point later in future)
	 */
	void setSubsequentNextRequested(int trainId,std::string lineId,int nextReq);

	/**
	 *Interface returns the platform name before the platform specified on the line specified
	 *@param lineId is the id of the line where the prePlatform is needed
	 *@param platformName is the name of the platform prior to which is the pre platform
	 *@return the pre platform
	 */
	std::string getPrePlatfrom(std::string lineId,std::string platformName) const;

	/**
	 *Interface that clears all the stop points of the train
	 *@param trainId is the id of the train whose stop points are to be cleared
	 *@param lineId is the id of the line where the train with given id specified is to be searched for
	 */
	void clearStopPoints(int trainId,std::string lineId);

	/**
	 * Interface that connects the trains after disruption(sets the next driver for trains)
	 *As the there will be no longer any looping and all trains in a line are connected in the order of their position in the train line
	 *@param lineId is the id of the line of interest
	 */
	void connectTrainsAfterDisruption(std::string lineId);

	private:
    /** mutex lock to lock the mapOfLineAndTrainDrivers **/
    mutable boost::mutex lineTrainDriversLock;
    /** is the map of line and the corresponding train drivers in it **/
    std::map<std::string, std::map<int,TrainDriver*> > mapOfLineAndTrainDrivers;
};
}
}
