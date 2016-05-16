//#pragma once

#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "boost/foreach.hpp"
#include "boost/regex.hpp"
#include "boost/thread/mutex.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
//#include "entities/params/PT_NetworkEntities.hpp"
#include "logging/Log.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"
//#include "PT_PathSetManager.hpp"
//#include "ServiceController.hpp"
//#include "SOCI_Converters.hpp"
#include "util/LangHelpers.hpp"
#include "entities/roles/driver/TrainDriver.hpp"
#include "entities/TrainController.hpp"
#include "entities/roles/driver/TrainDriverFacets.hpp"
#include <iostream>

//#ifndef _CLASS_TRAIN_CONTROLLER_FUNCTIONS
//#include "entities/TrainController.hpp"

using namespace std;
using namespace sim_mob;
using namespace luabridge;
using namespace medium;

ServiceController* ServiceController::pInstance=nullptr;
ServiceController::ServiceController()
{

}


double ServiceController::Use_ServiceController()
{
		LuaRef Use_Service_Controller = getGlobal(state.get(), "Use_Service_Controller");
		Use_Service_Controller(this);

}
ServiceController::~ServiceController()
{
}

 void ServiceController::mapClasses()
 {
	 getGlobalNamespace(state.get()).beginClass <ServiceController> ("ServiceController")
	 			.addFunction("reset_SpeedLimit",&ServiceController::resetSpeedLimit)
	 			.addFunction("reset_Acceleration",&ServiceController::resetAcceleration)
	 			.addFunction("reset_SafeHeadway_Sec",&ServiceController::resetSafeHeadwaySec)
	 			.addFunction("reset_SafeOperation_Distance",&ServiceController::resetSafeOperationDistance)
	 			.addFunction("reset_MovingCase", &ServiceController::resetMovingCase)
	 			.addFunction("force_Release_Passenegers", &ServiceController::forceReleasePassenegers)
				.addFunction("restrict_Passengers", &ServiceController::restrictPassengers)
				.addFunction("reset_HoldingTime_AtStation", &ServiceController::resetHoldingTimeAtStation)
				.addFunction("terminate_TrainService", &ServiceController::terminateTrainService)
				.addFunction("get_DwellTime", &ServiceController::getDwellTime)
	 			.endClass();

 }

 void ServiceController::resetSpeedLimit(double speedLimit,std::string lineId)
 {

	 std::vector<Block*> blockVector=TrainController<sim_mob::medium::Person_MT>::getInstance()->GetBlocks(lineId);
	 for (std::vector<Block*>::iterator it = blockVector.begin() ; it != blockVector.end(); ++it)
	 {
            (*it)->setSpeedLimit(speedLimit);
	 }
 }


 void ServiceController::resetAcceleration(double accelerate,std::string lineId)
 {
	 std::vector<Block*> blockVector=TrainController<sim_mob::medium::Person_MT>::getInstance()->GetBlocks(lineId);
	 	 for (std::vector<Block*>::iterator it = blockVector.begin() ; it != blockVector.end(); ++it)
	 	 {
	             (*it)->setAccelerateRate(accelerate);
	 	 }

 }

 void ServiceController::resetSafeHeadwaySec(double sec)
 {

 }

 void ServiceController::resetSafeOperationDistance(double distance)
 {

 }

 void ServiceController::resetMovingCase(int caseVal)
 {

 }

 //namespace
 void ServiceController::forceReleasePassenegers(int trainID,std::string lineId)
 {
	 map<std::string,std::vector<Role<Person_MT> *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	 	if(it != mapOfLineAndTrainDrivers.end())
	 	{
	 		std::vector<Role<sim_mob::medium::Person_MT>*> vect = it->second;
	 		for (std::vector<Role<sim_mob::medium::Person_MT>*>::iterator it = vect.begin() ; it != vect.end(); ++it)
	 		{
	 			Role<sim_mob::medium::Person_MT>* role=dynamic_cast<Role<sim_mob::medium::Person_MT>*>(*it);
	 			if(role)
	 			{
	 				//sim_mob::medium::TrainDriver *p;
	 				TrainDriver* driver= dynamic_cast<TrainDriver*>(role);
	 				if(driver)
	 				{
	 					driver->AlightAllPassengers();
	 				}
	 			}

	 		}

	 	}

 }

 void ServiceController::restrictPassengers(int behavior)
 {

 }
 void ServiceController::resetHoldingTimeAtStation(double time)
 {

 }

 void  ServiceController::terminateTrainService(std::string lineId)
 {
	 //Terminates train service


 }

 double  ServiceController::getDwellTime(int trainId,std::string lineId,std::string stationId)
 {
	 double waitingTime=-1;
	 map<std::string,std::vector<Role<Person_MT> *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	 if(it != mapOfLineAndTrainDrivers.end())
	 {
	 		std::vector<Role<sim_mob::medium::Person_MT>*> trainDrivers = it->second;
	 		for(std::vector<Role<sim_mob::medium::Person_MT>*>::iterator i=trainDrivers.begin();i!=trainDrivers.end();i++)
	 		{
	 			TrainDriver *tDriver=dynamic_cast<TrainDriver*>(*i);
	 			if(tDriver)
	 			{

	 				if(tDriver->getTrainId()==trainId&&tDriver->getTrainLine()==lineId)
	 				{
	 					Platform *platform=tDriver->getNextPlatform();
	 					std::string platformNo=platform->getPlatformNo();
	 					Station *station=TrainController<sim_mob::medium::Person_MT>::getInstance()->GetStationFromId(stationId);
	 					platform=station->getPlatform(lineId);
	 					if(platform)
	 					{
	 						if(boost::iequals(platformNo,platform->getPlatformNo()))
	 						{
	 							TrainMovement *trainMovement=tDriver->GetMovement();
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
		if(!pInstance) {
			pInstance = new ServiceController();
		}
		return pInstance;
	}

void ServiceController::InsertTrainIdAndTrainDriverInMap(int trainId,std::string lineId,Role<sim_mob::medium::Person_MT> *trainDriver)
{
	TrainIdLineId trainLineId;
	trainLineId.lineId=lineId;
	trainLineId.trainId=trainId;
	map<std::string,std::vector<Role<Person_MT> *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);

	if(it != mapOfLineAndTrainDrivers.end())
	{
	   //element found;
		std::vector<Role<sim_mob::medium::Person_MT>*> vect = it->second;
		vect.push_back(trainDriver);
		mapOfLineAndTrainDrivers[lineId]=vect;
	}
	else
	{
		std::vector<Role<sim_mob::medium::Person_MT>*> vect;
		vect.push_back(trainDriver);
		mapOfLineAndTrainDrivers[lineId]=vect;
	}
}

void ServiceController::RemoveTrainIdAndTrainDriverInMap(int trainId,std::string lineId,Role<sim_mob::medium::Person_MT> *trainDriver)
{
	map<std::string,std::vector<Role<Person_MT> *>>::iterator it=mapOfLineAndTrainDrivers.find(lineId);
	if(it != mapOfLineAndTrainDrivers.end())
	{
		std::vector<Role<sim_mob::medium::Person_MT>*> vect = it->second;
		int pos=-1;
		pos=std::find(vect.begin(), vect.end(), trainDriver)-vect.begin();
		if(pos>=0&&pos<=vect.size())
		{
			vect.erase(vect.begin()+pos);
			if(vect.size()==0)
				mapOfLineAndTrainDrivers.erase(lineId);
		}
	}
}

int ServiceController::GetTrainId(TrainDriver *trainDriver)
{

	return trainDriver->getTrainId();

}

std::string ServiceController::GetLineId(TrainDriver *trainDriver)
{
    return trainDriver->getTrainLine();
}

std::string ServiceController::GetOppositeLineId(std::string lineId)
{
	return TrainController<sim_mob::medium::Person_MT>::getInstance()->GetOppositeLineId(lineId);
}

std::string ServiceController::GetNextPlatform(TrainDriver *trainDriver)
{
      Platform *platform=trainDriver->getNextPlatform();
      return platform->getPlatformNo();
}

double ServiceController::GetDistanceToNextPlatform(TrainDriver *trainDriver)
{
      return trainDriver->GetMovement()->getDistanceToNextPlatform(trainDriver);

}

/*double ServiceController::SendTrainBackToDepot(TrainDriver *trainDriver)
{
	std::string lineId = (trainDriver)->getTrainLine();
	lastUsage[lineId] = false;
	removeAheadTrain(*it);
	(*it)->setNextRequested(TrainDriver::REQUESTED_TO_DEPOT);
    messaging::MessageBus::PostMessage(TrainController<Person_MT>::getInstance(),MSG_TRAIN_BACK_DEPOT, messaging::MessageBus::MessagePtr(new TrainMessage((*it)->getParent())));

}*/



