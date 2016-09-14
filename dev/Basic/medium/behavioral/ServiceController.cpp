//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "boost/foreach.hpp"
#include "boost/regex.hpp"
#include "boost/thread/mutex.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "logging/Log.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"
#include "util/LangHelpers.hpp"
#include "entities/roles/driver/TrainDriver.hpp"
#include "entities/TrainController.hpp"
#include "entities/TrainStationAgent.hpp"
#include "behavioral/ServiceController.hpp"

using namespace std;
using namespace sim_mob;
using namespace luabridge;
using namespace medium;

ServiceController* ServiceController::pInstance=nullptr;
ServiceController::ServiceController()
{


}


void ServiceController::useServiceController(std::string time)
{
	LuaRef useServiceControllerRef = getGlobal(state.get(), "use_servicecontroller");
	lineTrainDriversLock.lock();
	LuaRef retVal = useServiceControllerRef(this,time);
	lineTrainDriversLock.unlock();
}

ServiceController::~ServiceController()
{
}

 void ServiceController::mapClasses()
 {
	 getGlobalNamespace(state.get()).beginClass <ServiceController> ("ServiceController")
	 			.addFunction("reset_speed_limit",&ServiceController::resetSpeedLimit)
	 			.addFunction("reset_acceleration",&ServiceController::resetAcceleration)
	 			.addFunction("reset_safe_headway_sec",&ServiceController::resetSafeHeadwaySec)
	 			.addFunction("reset_safe_operation_distance",&ServiceController::resetSafeOperationDistance)
	 			.addFunction("reset_moving_case", &ServiceController::resetMovingCase)
	 			.addFunction("force_release_passenegers", &ServiceController::forceReleasePassenegers)
				.addFunction("restrict_passengers", &ServiceController::restrictPassengers)
				.addFunction("reset_holding_time_at_station", &ServiceController::resetHoldingTimeAtStation)
				.addFunction("terminate_trainservice", &ServiceController::terminateTrainService)
				.addFunction("get_dwelltime", &ServiceController::getDwellTime)
				.addFunction("get_opposite_lineid", &ServiceController::getOppositeLineId)
				.addFunction("get_lineid", &ServiceController::getLineId)
				.addFunction("get_distance_to_next_platform", &ServiceController::getDistanceToNextPlatform)
				.addFunction("get_next_platform", &ServiceController::getNextPlatform)
				.addFunction("get_active_trains", &ServiceController::getActiveTrainIds)
				.addFunction("get_active_trains_size", &ServiceController::getActiveTrainIdsSize)
				.addFunction("get_active_train_by_index", &ServiceController::getTrainIdByIndex)
				.addFunction("perform_disruption",&ServiceController::performDisruption)
				.addFunction("insert_stop_point",&ServiceController::insertStopPoint)
				.addFunction("update_platform_list",&ServiceController::updatePlatformList)
				.addFunction("terminate_single_train_service",&ServiceController::terminateTrainServiceForTrain)
				.addFunction("insert_unscheduled_train_trip",&ServiceController::insertUnScheduledTrain)
				.addFunction("set_disrupted_platforms",&ServiceController::setDisruptedPlatforms)
				.addFunction("get_disrupted_platforms",&ServiceController::getDisruptedPlatformByIndex)
				.addFunction("get_disrupted_platforms_size",&ServiceController::getDisruptedPlatformsSize)
				.addFunction("get_platform_by_offset",&ServiceController::getPlatformByOffset)
				.addFunction("set_uturn",&ServiceController::setUturnFlag)
				.addFunction("get_next_requested",&ServiceController::getNextRequestedMovementActionForTrain)
				.addFunction("get_trainid_train_ahead",&ServiceController::getTrainIdOfTrainAhead)
				.addFunction("set_ignore_safe_distance",&ServiceController::setIgnoreSafeDistance)
				.addFunction("set_ignore_safe_headway",&ServiceController::setIgnoreSafeHeadway)
				.addFunction("get_force_alight_status",&ServiceController::getForceAlightStatus)
				.addFunction("clear_disruption",&ServiceController::clearDisruption)
				.addFunction("set_force_alight_status",&ServiceController::setForceAlightStatus)
				.addFunction("is_stranded_during_disruption",&ServiceController::isStrandedDuringDisruption)
				.addFunction("set_subsequent_next_requested",&ServiceController::setSubsequentNextRequested)
				.addFunction("get_disrupted_state",&ServiceController::getDisruptedState)
				.addFunction("get_prev_platform",&ServiceController::getPrePlatfrom)
	 			.endClass();

 }

std::vector<TrainDriver*>  ServiceController::getActiveTrainsInLine(std::string lineId)
{
	std::vector <Role<Person_MT>*> activeTrains=TrainController<sim_mob::medium::Person_MT>::getInstance()->getActiveTrainsForALine(lineId);
	std::vector <Role<Person_MT>*>::iterator itr=activeTrains.begin();
	std::vector<TrainDriver*> trainDrivers;

    while(itr!=activeTrains.end())
    {
    	TrainDriver *driver=dynamic_cast<TrainDriver*>(*itr);
    	if(driver)
    	{
    		trainDrivers.push_back(driver);
    	}
    	itr++;
    }

    return trainDrivers;

}

std::string ServiceController::getPrePlatfrom(std::string lineId,std::string platformName) const
{
	Platform *platform=TrainController<sim_mob::medium::Person_MT>::getInstance()->getPrePlatform(lineId,platformName);
	return platform->getPlatformNo();
}


void ServiceController::setIgnoreSafeDistance(int trainId,std::string lineId,bool ignore)
{
	std::map<std::string,std::map<int,TrainDriver*>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		std::map<int,TrainDriver*>::iterator itr=mapOfTrainIdsVsDrivers.find(trainId);
		if(itr!=mapOfTrainIdsVsDrivers.end())
		{
			TrainDriver* driver = itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					TrainMovement *movement=driver->getMovement();
					if(movement)
					{
						movement->setIgnoreSafeDistanceByServiceController(ignore);
					}
				}
			}
		}
	}
}

void ServiceController::clearStopPoints(int trainId,std::string lineId)
{
	map<std::string,std::map<int,TrainDriver*>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr=mapOfTrainIdsVsDrivers.find(trainId);
		if(itr!=mapOfTrainIdsVsDrivers.end())
		{
			TrainDriver* driver= itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					//clear stop points
					driver->clearStopPoints();
				}
			}
		}
	}
}

void ServiceController::setIgnoreSafeHeadway(int trainId,std::string lineId,bool ignore)
{
	map<std::string,std::map<int,TrainDriver*>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr=mapOfTrainIdsVsDrivers.find(trainId);
		if(itr!=mapOfTrainIdsVsDrivers.end())
		{
			TrainDriver* driver= itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					TrainMovement *movement=driver->getMovement();
					if(movement)
					{
						movement->setIgnoreSafeHeadwayByServiceController(ignore);
					}
				}
			}
		}
	}
}

int ServiceController::getNextRequestedMovementActionForTrain(int trainId,std::string lineId) const
{
	int nextRequested=-1;
	std::map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr=mapOfTrainIdsVsDrivers.find(trainId);
		if(itr!=mapOfTrainIdsVsDrivers.end())
		{
			TrainDriver* driver= itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					nextRequested = driver->getNextRequested();
				}
			}
		}
	}
	return nextRequested;
}

void ServiceController::setSubsequentNextRequested(int trainId,std::string lineId,int nextReq)
{
	int nextRequested=-1;
	std::map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		std::map<int,TrainDriver*>::iterator itr=mapOfTrainIdsVsDrivers.find(trainId);
		if(itr!=mapOfTrainIdsVsDrivers.end())
		{
			TrainDriver* driver = itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					switch(nextReq)
					{
						case 1: //This is the number passed from lua script of service controller,1 means REQUESTED_AT_PLATFORM
						{
							driver->setSubsequentNextRequested(TrainDriver::REQUESTED_AT_PLATFORM);
							break;
						}
						case 5: //5 means REQUESTED_TO_PLATFROM
						{
							driver->setSubsequentNextRequested(TrainDriver::REQUESTED_TO_PLATFROM);
							break;
						}
						default:
							break;
					}

				}
			}
		}
	}
}

void ServiceController::setForceAlightStatus(int trainId,std::string lineId,bool status)
{
	map<std::string,std::map<int,TrainDriver*>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr=mapOfTrainIdsVsDrivers.find(trainId);
		if(itr!=mapOfTrainIdsVsDrivers.end())
		{
			TrainDriver* driver = itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					driver->setForceAlightStatus(status);
				}
			}
		}
	}
}

int ServiceController::getTrainIdOfTrainAhead(int trainId,std::string lineId) const
{
	int nextDrivertrainId=-1;
	std::map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		const std::map<int,TrainDriver*>::const_iterator itr=mapOfTrainIdsVsDrivers.find(trainId);
		if(itr!=mapOfTrainIdsVsDrivers.end())
		{
			TrainDriver* driver = itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					TrainDriver *nextDriver=driver->getNextDriver();
					if(nextDriver)
					{
						nextDrivertrainId = nextDriver->getTrainId();
					}
				}
			}
		}
	}
	return nextDrivertrainId;
}

void ServiceController::setUturnFlag(int trainId,std::string lineId,bool takeUturn)
{
	map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		std::map<int,TrainDriver*>::iterator itr=mapOfTrainIdsVsDrivers.find(trainId);
		if(itr!=mapOfTrainIdsVsDrivers.end())
		{
			TrainDriver* driver=itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					driver->setUturnFlag(takeUturn);

				}
			}
		}
	}
}

void ServiceController::insertStopPoint(int trainId,std::string lineId,double distance,double duration)
{
	map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr= mapOfTrainIdsVsDrivers.find(trainId);
		if(itr!=mapOfTrainIdsVsDrivers.end())
		{
			TrainDriver *driver=itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					TrainMovement *movement=driver->getMovement();
					if(movement)
					{
						const TrainPathMover &pathMover=movement->getPathMover();
						PolyPoint stopPoint=pathMover.GetStopPoint(distance);
						driver->insertStopPoint(stopPoint,duration);
					}
				}
			}
		}
	}
}

void ServiceController::insertUnScheduledTrain(std::string lineId,std::string startTime,std::string startStation)
{
	TrainController<sim_mob::medium::Person_MT>::getInstance()->composeTrainTripUnScheduled(lineId,startTime,startStation);
}

void ServiceController::restrictPassengers(std::string platformName,int trainId,std::string lineId,int type)
{
	map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr=mapOfTrainIdsVsDrivers.find(trainId);
		if(itr!=mapOfTrainIdsVsDrivers.end())
		{
			TrainDriver* driver= itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					driver->insertRestrictPassengerEntity(platformName,type);
				}
			}
		}
	}
}

void ServiceController::resetHoldingTimeAtStation(std::string platformName,double duration,int trainId,std::string lineId)
{
	map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		TrainDriver* driver = mapOfTrainIdsVsDrivers[trainId];
		if(driver)
		{
			if(driver->getTrainId()==trainId)
			{
				driver->insertPlatformHoldEntities(platformName,duration);
			}
		}
	}
}

void ServiceController::updatePlatformList(int trainId,LuaRef platformsToBeIgnored,std::string lineId)
{

	map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		TrainDriver* driver=mapOfTrainIdsVsDrivers[trainId];
		if(driver)
		{
			if(driver->getTrainId()==trainId)
			{

				if(platformsToBeIgnored.isTable())
				{
					int length=platformsToBeIgnored.length();
					std::vector<std::string> ignorePlatforms;
					for(int i=1;i<=length;i++)
					{
						ignorePlatforms.push_back(platformsToBeIgnored[i].cast<std::string>());
					}
					driver->AddPlatformsToIgnore(ignorePlatforms);
				}
			}
		}
	}
}

std::string ServiceController::getPlatformByOffset(int trainId,std::string lineId,int offset) const
{
	std::string offsetPlatform="";
	std::map<std::string,std::map<int,TrainDriver*>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it!=mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &trainsInLine=it->second;
		std::map<int,TrainDriver*>::const_iterator itr=trainsInLine.find(trainId);
		if(itr!=trainsInLine.end())
		{
			TrainDriver *driver=itr->second;
			if(driver&&driver->getTrainId()==trainId)
			{
				TrainMovement *trainMovement=driver->getMovement();
				if(trainMovement)
				{

					offsetPlatform=trainMovement->getPlatformByOffset(offset);
				}
			}
		}

	}
	return offsetPlatform;
}

void ServiceController::performDisruption(std::string startStation,std::string endStation,std::string time)
{
	TrainController<sim_mob::medium::Person_MT>::getInstance()->setDisruptionParams(startStation,endStation,time);
}

void ServiceController::setDisruptedPlatforms(std::string startStation,std::string endStation,std::string lineID)
{
	TrainController<sim_mob::medium::Person_MT>::getInstance()->setDisruptedPlatforms(startStation,endStation,lineID);
}

std::string ServiceController::getDisruptedPlatformByIndex(std::string lineID,int index) const
{
	const std::map<std::string,std::vector<std::string>> &platforms=TrainController<sim_mob::medium::Person_MT>::getInstance()->getDisruptedPlatforms_ServiceController();
	std::map<std::string,std::vector<std::string>>::const_iterator itr= platforms.find(lineID);
	if(itr!=platforms.end())
	{
		const std::vector<std::string> &platformNames=itr->second;
		if(!platformNames.empty()&&index>=0&&index<platformNames.size())
		{
			return platformNames.at(index);
		}
	}

	return "";
}

void ServiceController::clearDisruption(std::string lineId)
{
	TrainController<sim_mob::medium::Person_MT>::getInstance()->clearDisruption(lineId);
	//connect the trains to break the uTurn loop.
	connectTrainsAfterDisruption(lineId);
}

void ServiceController::addTrainIdToInactivePoolOnJourneyCompletion(int trainId,std::string lineId)
{
	map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> &mapOfIdsVsTrainDrivers = it->second;
		std::map<int,TrainDriver*>::iterator itr=mapOfIdsVsTrainDrivers.find(trainId);
		if(itr!=mapOfIdsVsTrainDrivers.end())
		{
			TrainDriver* driver=itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					TrainController<sim_mob::medium::Person_MT>::getInstance()->pushToInactivePoolAfterTripCompletion(trainId,lineId);
				}
			}
		}
	}
}

void ServiceController::connectTrainsAfterDisruption(std::string lineId)
{
	std::map<std::string,std::map<int,TrainDriver*>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	std::vector<TrainDriver*> trainDrivers;
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		for(std::map<int,TrainDriver*>::iterator itr=mapOfTrainIdsVsDrivers.begin();itr!=mapOfTrainIdsVsDrivers.end();itr++)
		{
			trainDrivers.push_back(itr->second);
		}
		std::sort(trainDrivers.begin(),trainDrivers.end());
		//after sorting connect
		for(std::vector<TrainDriver*>::iterator it=trainDrivers.begin();it!=trainDrivers.end();it++)
		{
			TrainDriver *driver=(*it);
			int id=driver->getTrainId();
			if(driver->getNextDriver()!=nullptr)
			{
				TrainDriver *dr=*(it+1);
				if(driver->getNextDriver()!=dr)
				{
					driver->setNextDriver(dr);
				}
			}
			else
			{
				if((it+1)!=trainDrivers.end())
				{
					driver->setNextDriver(*(it+1));
				}
			}
		}

		//set the last driver for the first station agent so the subsequent trains coming can be connected to it as next driver
		std::vector<Platform*> platforms;
		TrainController<sim_mob::medium::Person_MT>::getInstance()->getTrainPlatforms(lineId, platforms);
		if(platforms.size()>0)
		{
			Platform *platform=*(platforms.begin());

			if(platform)
			{
				Agent *stationAgent=TrainController<sim_mob::medium::Person_MT>::getInstance()->getAgentFromStation(platform->getStationNo());
				TrainStationAgent *trainStationAgent=dynamic_cast<TrainStationAgent*>(stationAgent);
				if(trainStationAgent)
				{
					trainStationAgent->setLastDriver(lineId,(*trainDrivers.begin()));
				}
			}
		}
	}
}

int ServiceController::getDisruptedPlatformsSize(std::string lineID) const
{
	std::map<std::string,std::vector<std::string>> platforms=TrainController<sim_mob::medium::Person_MT>::getInstance()->getDisruptedPlatforms_ServiceController();
	if(platforms.find(lineID)!=platforms.end())
	{
		const std::vector<std::string>& platformNames=platforms[lineID];
		return platformNames.size();
	}
	return 0;
}
std::vector<int>   ServiceController::getActiveTrainIds(std::string lineId)
{
	std::vector<int> trainIds;
	lineTrainDriversLock.lock();
	map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it!=mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*>trainsInLine=mapOfLineAndTrainDrivers[lineId];
		std::map<int,TrainDriver *>::iterator itr= trainsInLine.begin();
		while(itr!=trainsInLine.end())
		{
			TrainDriver *driver=(itr)->second;
			if(driver)
			{
				trainIds.push_back(driver->getTrainId());
			}
			itr++;
		}
	}

	lineTrainDriversLock.unlock();
	return trainIds;
}

int ServiceController::getActiveTrainIdsSize(const std::string lineId) const
{
	map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it!=mapOfLineAndTrainDrivers.end())
	{
		const map<int,TrainDriver *> &trainDrivers=it->second;
		return trainDrivers.size();
	}
	return 0;
}

int ServiceController::getTrainIdByIndex(int index,std::string lineId) const
{
	int trainId=-1;
	map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it!=mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver *>&trainsInLine=it->second;
		std::map<int,TrainDriver *>::const_iterator itr= trainsInLine.begin();
		if(itr!=trainsInLine.end())
		{
			std::advance(itr,index);
			TrainDriver *driver=itr->second;
			if(driver)
			{
				return driver->getTrainId();
			}
		}
	}
	return -1;
}

 void ServiceController::resetSpeedLimit(double speedLimit,std::string startStation,std::string endStation,std::string lineId,std::string startTime,std::string endTime)
 {

	 ResetBlockSpeeds resetSpeedBlocks;
	 resetSpeedBlocks.endStation=endStation;
	 resetSpeedBlocks.startStation=startStation;
	 resetSpeedBlocks.line = lineId;
	 resetSpeedBlocks.speedReset=false;
	 resetSpeedBlocks.speedLimit=speedLimit;
	 resetSpeedBlocks.startTime=startTime;
	 resetSpeedBlocks.endTime=endTime;

	 //Pass the message
	 TrainController<Person_MT>::getInstance()->assignResetBlocks(resetSpeedBlocks);
	 //messaging::MessageBus::PostMessage(TrainController<Person_MT>::getInstance(),
	//		 84099002, messaging::MessageBus::MessagePtr(new ResetSpeedMessage(resetSpeedBlocks)));

 }
 bool ServiceController::isStrandedDuringDisruption(int trainId,std::string lineId) const
 {
	std::map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &mapOfTrainIdsVsDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr=mapOfTrainIdsVsDrivers.find(trainId);
		if(itr!=mapOfTrainIdsVsDrivers.end())
		{
			TrainDriver* driver=itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					TrainMovement *trainMovement=driver->getMovement();
					if(trainMovement)
					{
						return trainMovement->isStrandedBetweenPlatform();
					}
				}
			}
		}
	}
	return false;
}

bool ServiceController::getDisruptedState(int trainId,std::string lineId) const
{
	std::map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &mapOfIdsVsTrainDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr=mapOfIdsVsTrainDrivers.find(trainId);
		if(itr!=mapOfIdsVsTrainDrivers.end())
		{
			TrainDriver* driver= itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					TrainMovement *trainMovement=driver->getMovement();
					if(trainMovement)
					{
						return trainMovement->getDisruptedState();
					}
				}
			}
		}
	}
	return false;
}
 void ServiceController::resetAcceleration(double accelerate,std::string lineId)
 {
	 std::vector<Block*> blockVector=TrainController<sim_mob::medium::Person_MT>::getInstance()->getBlocks(lineId);
	 for (std::vector<Block*>::iterator it = blockVector.begin() ; it != blockVector.end(); ++it)
	 {
			 (*it)->setAccelerateRate(accelerate);
	 }
 }

 void ServiceController::resetSafeHeadwaySec(double sec,int trainId,std::string lineId)
 {
	map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> &mapOfIdsVsTrainDrivers = it->second;
		std::map<int,TrainDriver*>::iterator itr=mapOfIdsVsTrainDrivers.find(trainId);
		if(itr!=mapOfIdsVsTrainDrivers.end())
		{
			TrainDriver* driver=itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					TrainMovement *trainMovement=driver->getMovement();
					if(trainMovement)
					{
						trainMovement->resetSafeHeadWay(sec);
					}
				}
			}
		}
	}
 }

 void ServiceController::resetSafeOperationDistance(double distance,int trainId,std::string lineId)
 {
	map<std::string,std::map<int,TrainDriver *>>::iterator itr=mapOfLineAndTrainDrivers.find(lineId);
	if(itr != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> &mapOfIdsVsTrainDrivers = itr->second;
		std::map<int,TrainDriver*>::iterator it=mapOfIdsVsTrainDrivers.find(trainId);
		if(it!=mapOfIdsVsTrainDrivers.end())
		{
			TrainDriver* driver=it->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					TrainMovement *trainMovement=driver->getMovement();
					if(trainMovement)
					{
						trainMovement->resetSafeDistance(distance);

					}
				}
			}
		}
	}
 }

 void ServiceController::resetMovingCase(int trainId,std::string lineId,int caseVal)
 {
	map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &mapOfIdsVsTrainDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr=mapOfIdsVsTrainDrivers.find(trainId);
		if(itr!=mapOfIdsVsTrainDrivers.end())
		{
			TrainDriver* driver=itr->second;
			if(driver)
			{
				if(driver->getTrainId()==trainId)
				{
					TrainMovement *movement=driver->getMovement();
					if(movement)
					{
						std::string platformNo=movement->getNextPlatform()->getPlatformNo();
						movement->resetMovingCase(TRAINCASE(caseVal));
					}
				}
			}
		}
	}
 }

 //namespace

bool ServiceController::getForceAlightStatus(int trainID,std::string lineId) const
 {
	std::map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*>& trainDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr=trainDrivers.find(trainID);
		if(itr!=trainDrivers.end())
		{
			TrainDriver *driver=itr->second;
			if(driver&&driver->getTrainId()==trainID)
			{
				return driver->getForceAlightStatus();
			}
		}
	}
	return false;
 }

void ServiceController::forceReleasePassenegers(int trainID,std::string lineId,bool action)
{
	std::map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &mapOfIdsVsTrainDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr=mapOfIdsVsTrainDrivers.find(trainID);
		if(itr!=mapOfIdsVsTrainDrivers.end())
		{
			TrainDriver* driver= itr->second;
			if(driver&&driver->getTrainId()==trainID)
			{
				driver->setForceAlightFlag(action);
			}
		}
	}
}

 void  ServiceController::terminateTrainService(std::string lineId)
 {
	 //Terminates train service
	TrainController<sim_mob::medium::Person_MT>::getInstance()->terminateTrainService(lineId); //stops the dispatch of new train
	map<std::string,std::map<int,TrainDriver*>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> trainDrivers = it->second;
		std::map<int,TrainDriver*>::iterator itr=trainDrivers.begin();
		while(itr!=trainDrivers.end())
		{
			TrainDriver *trainDriver=itr->second;
			if(trainDriver)
			{
				trainDriver->setTerminateTrainService(true);
			}
			itr++;
		}
	 }
 }

double  ServiceController::getDwellTime(int trainId,std::string lineId,std::string stationId) const
{
	double waitingTime=-1;
	map<std::string,std::map<int,TrainDriver *>>::const_iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		const std::map<int,TrainDriver*> &trainDrivers = it->second;
		std::map<int,TrainDriver*>::const_iterator itr=trainDrivers.find(trainId);
		if(itr!=trainDrivers.end())
		{
			TrainDriver *tDriver=itr->second;
			if(tDriver)
			{

				if(tDriver->getTrainId()==trainId&&tDriver->getTrainLine()==lineId)
				{
					Platform *platform=tDriver->getNextPlatform();
					std::string platformNo=platform->getPlatformNo();
					Station *station=TrainController<sim_mob::medium::Person_MT>::getInstance()->getStationFromId(stationId);
					platform=station->getPlatform(lineId);
					if(platform)
					{
						if(boost::iequals(platformNo,platform->getPlatformNo()))
						{
							TrainMovement *trainMovement=tDriver->getMovement();
							if(trainMovement)
							{
								if(trainMovement->getDistanceToNextPlatform(tDriver)==0)
								{
									waitingTime=tDriver->getWaitingTime();

								}
							}
						}
					}
				}
			}
		}
	}
	return waitingTime;
}

 ServiceController * ServiceController::getInstance()
{
	if(!pInstance)
	{
		pInstance = new ServiceController();
	}
	return pInstance;
}

void ServiceController::insertTrainIdAndTrainDriverInMap(int trainId,std::string lineId,TrainDriver *trainDriver)
{
	TrainIdLineId trainLineId;
	trainLineId.lineId=lineId;
	trainLineId.trainId=trainId;
	lineTrainDriversLock.lock();
	map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
	   //element found;
		std::map<int,TrainDriver*> &trainDrivers = it->second;
		trainDrivers[trainId]=trainDriver;
	}
	else
	{
		map<int,TrainDriver*> trainDrivers;
		trainDrivers[trainId]=trainDriver;
		mapOfLineAndTrainDrivers[lineId]=trainDrivers;
	}
	lineTrainDriversLock.unlock();
}

void ServiceController::removeTrainIdAndTrainDriverInMap(int trainId,std::string lineId,TrainDriver *trainDriver)
{
	lineTrainDriversLock.lock();
	std::map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> &trainDrivers = it->second;
		int pos=-1;
		std::map<int,TrainDriver*>::iterator itr=trainDrivers.find(trainId);
		if(itr!=trainDrivers.end())
		{
			trainDrivers.erase(trainId);
			if(trainDrivers.size()==0)
			{
				mapOfLineAndTrainDrivers.erase(lineId);
			}
		}
	}
	lineTrainDriversLock.unlock();
}

int ServiceController::getTrainId(TrainDriver *trainDriver)
{
	return trainDriver->getTrainId();
}

std::string ServiceController::getLineId(TrainDriver *trainDriver)
{
    return trainDriver->getTrainLine();
}

std::string ServiceController::getOppositeLineId(std::string lineId)
{
	return TrainController<sim_mob::medium::Person_MT>::getInstance()->getOppositeLineId(lineId);
}

std::string ServiceController::getNextPlatform(int trainId,std::string lineId)
{

	map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	std::string nextPlatform="";
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*>& trainDrivers = it->second;
		std::map<int,TrainDriver*>::iterator itr=trainDrivers.find(trainId);
		if(itr!=trainDrivers.end())
		{
			TrainDriver *tDriver=itr->second;
			if(tDriver)
			{
				if(tDriver->getTrainId()==trainId&&tDriver->getTrainLine()==lineId)
				{
				   Platform *platform=tDriver->getNextPlatform();
				   return platform->getPlatformNo();

				}
			}
		}
	}

	return nextPlatform;
}

double ServiceController::getDistanceToNextPlatform(std::string lineId,int trainId)
{
	std::map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	double distance=-1.0;
	 if(it != mapOfLineAndTrainDrivers.end())
	 {
		std::map<int,TrainDriver*> trainDrivers = it->second;
		std::map<int,TrainDriver*>::iterator itr=trainDrivers.find(trainId);
		if(itr!=trainDrivers.end())
		{
			TrainDriver *tDriver = itr->second;
			if(tDriver)
			{
				if(tDriver->getTrainId()==trainId&&tDriver->getTrainLine()==lineId)
				{
					Platform *platform=tDriver->getNextPlatform();
					if(platform)
					{
					   TrainMovement *trainMovement=tDriver->getMovement();
					   if(trainMovement)
					   {
						   return trainMovement->getDistanceToNextPlatform(tDriver);

					   }

					}
				}
			}
		}
	 }

return distance;
}


void ServiceController::pushTrainIntoInActivePool(int trainID,std::string lineID)
{
	TrainController<sim_mob::medium::Person_MT>::getInstance()->pushTrainIntoInActivePool(trainID,lineID);
}

void ServiceController::pullOutTrainFromInActivePool(std::string lineID)
{
	TrainController<sim_mob::medium::Person_MT>::getInstance()->pullOutTrainFromInActivePool(lineID);
}

void ServiceController::terminateTrainServiceForTrain(int trainId,std::string lineId)
{

	map<std::string,std::map<int,TrainDriver *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
    if(it != mapOfLineAndTrainDrivers.end())
	{
		std::map<int,TrainDriver*> trainDrivers = it->second;
		std::map<int,TrainDriver*>::iterator itr=trainDrivers.find(trainId);
		if(itr!=trainDrivers.end())
		{
			TrainDriver *trainDriver=itr->second;
			if(trainDriver)
			{
				if(trainDriver->getTrainId()==trainId)
				{
					trainDriver->setTerminateTrainService(true);
				}
			}
		}
	}
}






