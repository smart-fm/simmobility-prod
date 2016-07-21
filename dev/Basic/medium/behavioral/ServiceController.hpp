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

//#include "Path.hpp"
//#include "soci/soci.h"
using namespace std;
namespace sim_mob
{

namespace medium
{
//class TrainMovement;
struct TrainIdLineId
{
	int trainId;
	std::string lineId;
};

/*enum passengerMovement
{
	BOARDING=0,
	ALIGHTING,
	BOTH
};*/

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
	void Use_ServiceController(std::string time);

	void SetParameters()
	{

		//set the values for parameters here
	}

	void AddParameters()
	{
          //add new parameters to lua files
	}

	void resetSpeedLimit(double speedLimit,std::string startStation,std::string endStation,std::string lineId,std::string startTime,std::string endTime);
	void resetAcceleration(double accelerate,std::string lineId);
	void resetSafeHeadwaySec(double sec,int trainId,std::string lineId);
	void resetSafeOperationDistance(double distance,int trainId,std::string lineId);
	void resetMovingCase(int caseVal);
	void forceReleasePassenegers(int trainId,std::string lineId,bool action);
	void resetHoldingTimeAtStation(std::string platformName,double duration,int trainId,std::string lineId);
	void terminateTrainService(std::string lineId);
	void Uturn(int trainId,std::string lineId);
	double getDwellTime(int trainId,std::string lineId,std::string station);
	static ServiceController* getInstance();
	void mapClasses();

	map<std::string, std::vector<Role<sim_mob::medium::Person_MT>*> > mapOfLineAndTrainDrivers;
    void InsertTrainIdAndTrainDriverInMap(int trainId,std::string lineId,Role<sim_mob::medium::Person_MT> *trainDriver);
    void RemoveTrainIdAndTrainDriverInMap(int trainId,std::string lineId,Role<sim_mob::medium::Person_MT> *trainDriver);
    int GetTrainId(TrainDriver *trainDriver);
    std::string GetLineId(TrainDriver *trainDriver);
    std::string GetOppositeLineId(std::string lineId);
    std::string GetNextPlatform(int trainId,std::string lineID);
    double GetDistanceToNextPlatform(std::string lineId,int trainId);

    void PushTrainIntoInActivePool(int trainID,std::string lineID);
    void pullOutTrainFromInActivePool(std::string lineID);
    void TerminateTrainServiceForTrain(int trainId,std::string lineId);
    std::vector<TrainDriver*> GetActiveTrainsInLine(std::string lineId);
    std::vector<int>  GetActiveTrainIds(std::string lineId);
    int GetActiveTrainIdsSize(std::string lineId);
    int GetTrainIdByIndex(int index,std::string lineId);
    void PerformDisruption(std::string startStation,std::string endStation,std::string time);
    void InsertStopPoint(int trainId,std::string lineId,double distance,double duration);
    void UpdatePlatformList(int trainId,LuaRef platformsToBeIgnored,std::string lineId);
    void InsertPlatformHoldEntities(std::string platformName,double duration,int trainId,std::string lineId);
    void restrictPassengers(std::string platformName,int trainId,std::string lineId,int type);
    void InsertUnScheduledTrain(std::string lineId,std::string startTime,std::string startStation);
    void SetDisruptedPlatforms(std::string startStation,std::string endStation,std::string lineID);
    std::string GetPlatformByOffset(int trainId,std::string lineId,int offset);
    std::string GetDisruptedPlatformByIndex(std::string lineID,int index);
    int GetDisruptedPlatformsSize(std::string lineID);
    void SetUnsetUturnFlag(int trainId,std::string lineId,bool takeUturn);
    int getTrainIdOfTrainAhead(int trainId,std::string lineId);
    int getNextRequestedForTrain(int trainId,std::string lineId);
    void setUnsetIgnoreSafeDistance(int trainId,std::string lineId,bool ignore);
    void setUnsetIgnoreSafeHeadway(int trainId,std::string lineId,bool ignore);
    void ClearDisruption(std::string lineId);
    bool getForceAlightStatus(int trainID,std::string lineId);
    void setUnsetForceAlightStatus(int trainId,std::string lineId,bool status);
    bool getDisruptedState(int trainId,std::string lineId);
    bool IsStrandedDuringDisruption(int trainId,std::string lineId);
    void setSubsequentNextRequested(int trainId,std::string lineId,int nextReq);
    //void setUnsetForceAlightPassengers(int trainId,std::string lineId,bool foreAlight)

	private:
    mutable boost::mutex lineTrainDriversLock;
//TrainIdLineId TrainLtrainLineId,Role<sim_mob::Person>*trainDriver

};
}
}
