#pragma once

#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include "entities/misc/PublicTransit.hpp"
#include "lua/LuaModel.hpp"
#include "entities/roles/Role.hpp"
#include "entities/Person_MT.hpp"

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

class ServiceController:  public lua::LuaModel
{
	public:

	static ServiceController* pInstance;
	explicit ServiceController();
	virtual ~ServiceController();
	double Use_ServiceController();

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
	void forceReleasePassenegers(int trainId,std::string lineId);
	void restrictPassengers(int behavior);
	void resetHoldingTimeAtStation(double time);
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
    std::string GetNextPlatform(TrainDriver *trainDriver);
    double GetDistanceToNextPlatform(TrainDriver * trainDriver);
//TrainIdLineId TrainLtrainLineId,Role<sim_mob::Person>*trainDriver

};
}
}
