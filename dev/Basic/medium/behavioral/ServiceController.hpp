/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 *
 * File:   ServiceController.hpp
 * Author: Jabir <jabir@smart.mit.edu>
 *
 */

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

using namespace std;
namespace sim_mob
{

namespace medium
{

struct TrainIdLineId
{
	int trainId;
	std::string lineId;
};

using namespace std;
using namespace sim_mob;
using namespace luabridge;
using namespace medium;

class ServiceController:  public lua::LuaModel
{
	public:
	static ServiceController* pInstance;
	explicit ServiceController();
	virtual ~ServiceController();
	void mapClasses();
	void useServiceController(std::string time);
	void resetSpeedLimit(double speedLimit,std::string startStation,std::string endStation,std::string lineId,std::string startTime,std::string endTime);
	void resetAcceleration(double accelerate,std::string lineId);
	void resetSafeHeadwaySec(double sec,int trainId,std::string lineId);
	void resetSafeOperationDistance(double distance,int trainId,std::string lineId);
	void resetMovingCase(int trainId,std::string lineId,TRAINCASE caseVal);
	void forceReleasePassenegers(int trainId,std::string lineId,bool action);
	void resetHoldingTimeAtStation(std::string platformName,double duration,int trainId,std::string lineId);
	void terminateTrainService(std::string lineId);
	double getDwellTime(int trainId,std::string lineId,std::string station);
	static ServiceController* getInstance();
    void insertTrainIdAndTrainDriverInMap(int trainId,std::string lineId,TrainDriver *trainDriver);
    void removeTrainIdAndTrainDriverInMap(int trainId,std::string lineId,TrainDriver *trainDriver);
    int getTrainId(TrainDriver *trainDriver);
    std::string getLineId(TrainDriver *trainDriver);
    std::string getOppositeLineId(std::string lineId);
    std::string getNextPlatform(int trainId,std::string lineID);
    double getDistanceToNextPlatform(std::string lineId,int trainId);
    void pushTrainIntoInActivePool(int trainID,std::string lineID);
    void pullOutTrainFromInActivePool(std::string lineID);
    void terminateTrainServiceForTrain(int trainId,std::string lineId);
    std::vector<TrainDriver*> getActiveTrainsInLine(std::string lineId);
    std::vector<int>  getActiveTrainIds(std::string lineId);
    int getActiveTrainIdsSize(std::string lineId);
    int getTrainIdByIndex(int index,std::string lineId);
    void performDisruption(std::string startStation,std::string endStation,std::string time);
    void insertStopPoint(int trainId,std::string lineId,double distance,double duration);
    void updatePlatformList(int trainId,LuaRef platformsToBeIgnored,std::string lineId);
    void insertPlatformHoldEntities(std::string platformName,double duration,int trainId,std::string lineId);
    void restrictPassengers(std::string platformName,int trainId,std::string lineId,int type);
    void insertUnScheduledTrain(std::string lineId,std::string startTime,std::string startStation);
    void setDisruptedPlatforms(std::string startStation,std::string endStation,std::string lineID);
    std::string getPlatformByOffset(int trainId,std::string lineId,int offset);
    std::string getDisruptedPlatformByIndex(std::string lineID,int index);
    int getDisruptedPlatformsSize(std::string lineID);
    void setUnsetUturnFlag(int trainId,std::string lineId,bool takeUturn);
    int getTrainIdOfTrainAhead(int trainId,std::string lineId);
    int getNextRequestedForTrain(int trainId,std::string lineId);
    void setUnsetIgnoreSafeDistance(int trainId,std::string lineId,bool ignore);
    void setUnsetIgnoreSafeHeadway(int trainId,std::string lineId,bool ignore);
    void clearDisruption(std::string lineId);
    bool getForceAlightStatus(int trainID,std::string lineId);
    void setUnsetForceAlightStatus(int trainId,std::string lineId,bool status);
    bool getDisruptedState(int trainId,std::string lineId);
    bool isStrandedDuringDisruption(int trainId,std::string lineId);
    void setSubsequentNextRequested(int trainId,std::string lineId,int nextReq);
    std::string getPrePlatfrom(std::string lineId,std::string platformName);
    void clearStopPoints(int trainId,std::string lineId);

	private:
    mutable boost::mutex lineTrainDriversLock;
    map<std::string, std::vector<TrainDriver*> > mapOfLineAndTrainDrivers;
};
}
}
