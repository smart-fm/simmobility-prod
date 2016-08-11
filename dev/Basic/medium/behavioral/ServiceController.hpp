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
 *
 * class:  ServiceController.hpp
 * Author: Jabir <jabir@smart.mit.edu>
 *
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
	/*This resets the speed limit of a stretch of blocks  */
	void resetSpeedLimit(double speedLimit,std::string startStation,std::string endStation,std::string lineId,std::string startTime,std::string endTime);
	/*This resets the acceleration of a stretch of blocks*/
	void resetAcceleration(double accelerate,std::string lineId);
	/*This resets the safe headway of a particular train */
	void resetSafeHeadwaySec(double sec,int trainId,std::string lineId);
	/*This resets the safe operation distance of a particular train  */
	void resetSafeOperationDistance(double distance,int trainId,std::string lineId);
	/* This resets operation of train from station case to normal case and from normal case to station case */
	void resetMovingCase(int trainId,std::string lineId,TRAINCASE caseVal);
	/*This force releases all the passengers of the train when it approaches the nearest  station */
	void forceReleasePassenegers(int trainId,std::string lineId,bool action);
	/*This resets or overrides the default waiting time of a train at station if the holding time is more than calculated waiting time */
	void resetHoldingTimeAtStation(std::string platformName,double duration,int trainId,std::string lineId);
	/*This terminates the service of all trains currently running on the line specified and stops the future schedule dispatch of trains*/
	void terminateTrainService(std::string lineId);
	/* gets the waiting time remaining of the train if the train is at the station specified */
	double getDwellTime(int trainId,std::string lineId,std::string station);
	static ServiceController* getInstance();
	/*This inserts the train into the map of service controller */
    void insertTrainIdAndTrainDriverInMap(int trainId,std::string lineId,TrainDriver *trainDriver);
    /* removes the train from the map of service controller */
    void removeTrainIdAndTrainDriverInMap(int trainId,std::string lineId,TrainDriver *trainDriver);
    /*gets the id of the train from the trainDriver passed */
    int getTrainId(TrainDriver *trainDriver);
    /* gets the lineId of the train from the train driver passed */
    std::string getLineId(TrainDriver *trainDriver);
    /* gets the lineID of the opposite line eg for NE_1 it gives NE_2 and vice versa */
    std::string getOppositeLineId(std::string lineId);
    /*This gives the nearest coming platform for a train*/
    std::string getNextPlatform(int trainId,std::string lineID);
    /*This returns the distance to coming platform for the train specified */
    double getDistanceToNextPlatform(std::string lineId,int trainId);
    /*This pushes the train into inactive pool from active pool */
    void pushTrainIntoInActivePool(int trainID,std::string lineID);
    /* This pulls out the train from inactive pool to active pool */
    void pullOutTrainFromInActivePool(std::string lineID);
    /* This terminates the service for the entire train */
    void terminateTrainServiceForTrain(int trainId,std::string lineId);
    /* This gives all the active trains in the line */
    std::vector<TrainDriver*> getActiveTrainsInLine(std::string lineId);
    /* This gives the ids of all active trains in line */
    std::vector<int>  getActiveTrainIds(std::string lineId);
    /* This gives the number of active trains in the line */
    int getActiveTrainIdsSize(std::string lineId);
    /* This gives the id of the train by index in the list */
    int getTrainIdByIndex(int index,std::string lineId);
    /* This is the direct Api to perform disruption */
    void performDisruption(std::string startStation,std::string endStation,std::string time);
    /*This inserts stop point for a train on its path at a distance specified */
    void insertStopPoint(int trainId,std::string lineId,double distance,double duration);
    /*This updates the platform list,basically currently it just specifies the platforms to be ignored as there is only one line
     * and by default train stops at all platforms */
    void updatePlatformList(int trainId,luabridge::LuaRef platformsToBeIgnored,std::string lineId);
    void insertPlatformHoldEntities(std::string platformName,double duration,int trainId,std::string lineId);
    /*This restricts the passenger movement type.ie boarding,alighting or both*/
    void restrictPassengers(std::string platformName,int trainId,std::string lineId,int type);
    /*This inserts an unscheduled train on the line,the starting point can be at any platform */
    void insertUnScheduledTrain(std::string lineId,std::string startTime,std::string startStation);
    /* This sets the disruption on the platform */
    void setDisruptedPlatforms(std::string startStation,std::string endStation,std::string lineID);
    /* This gives the platform by offset that is the platform the train will arrive after skipping the
     * number of platforms specified ,offset 0 means the coming platform*/
    std::string getPlatformByOffset(int trainId,std::string lineId,int offset);
    /* this gives the index of disrupted platform by index specified */
    std::string getDisruptedPlatformByIndex(std::string lineID,int index);
    /* This returns the number of disrupted platforms */
    int getDisruptedPlatformsSize(std::string lineID);
    /* This sets or unsets the Uturn signal of the train */
    void setUnsetUturnFlag(int trainId,std::string lineId,bool takeUturn);
    /* This gives the trainId of the train ahead of the train specified */
    int getTrainIdOfTrainAhead(int trainId,std::string lineId);
    /* This gives the future next requested for the train */
    int getNextRequestedMovementActionForTrain(int trainId,std::string lineId);
    /* This sets or clears the ignore safe distance flag for the train */
    void setIgnoreSafeDistance(int trainId,std::string lineId,bool ignore);
    /* This sets or clears the ignore safe headway flag for the train */
    void setUnsetIgnoreSafeHeadway(int trainId,std::string lineId,bool ignore);
    /*This clears disruption on the platforms */
    void clearDisruption(std::string lineId);
    /* this gets the force alight status that is passengers have force alighted or not */
    bool getForceAlightStatus(int trainID,std::string lineId);
    /*This sets or unsets force alight status */
    void setUnsetForceAlightStatus(int trainId,std::string lineId,bool status);
    /* This returns whether the train is in disrupted region or not */
    bool getDisruptedState(int trainId,std::string lineId);
    /*This returns if the train is stranded between platform in disrupted region or not */
    bool isStrandedDuringDisruption(int trainId,std::string lineId);
    /* This sets the subsequent next requested for the train */
    void setSubsequentNextRequested(int trainId,std::string lineId,int nextReq);
    /* This returns the platform name before the platform specified on the line specified */
    std::string getPrePlatfrom(std::string lineId,std::string platformName);
    /* This clears all the stop points of the train */
    void clearStopPoints(int trainId,std::string lineId);
    /* connects trains after disruption */
    void connectTrainsAfterDisruption(std::string lineId);

	private:
    mutable boost::mutex lineTrainDriversLock;
    std::map<std::string, std::vector<TrainDriver*> > mapOfLineAndTrainDrivers;
};
}
}
