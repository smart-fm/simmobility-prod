/*
 * TrainRemoval.cpp
 *
 *  Created on: 3 Sep 2016
 *      Author: jabir
 */

#include <entities/TrainRemoval.hpp>
#include "entities/TrainController.hpp"
#include "entities/Person_MT.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/driver/TrainPathMover.hpp"
#include "entities/TrainStationAgent.hpp"
#include "entities/roles/driver/TrainDriverFacets.hpp"
#include "behavioral/ServiceController.hpp"
using namespace std;
namespace sim_mob
{
namespace medium
{

	TrainRemoval *TrainRemoval::instance =nullptr;
	TrainRemoval::TrainRemoval()
	{

	}

	TrainRemoval *TrainRemoval::getInstance()
	{
		if(!instance)
		{
			instance= new TrainRemoval();
		}
		return instance;
	}

	TrainRemoval::~TrainRemoval()
	{

	}

	void TrainRemoval::addToTrainRemovalList(TrainDriver *trainDriver)
	{
		trainRemovalListLock.lock();
		trainsToBeRemoved.push_back(trainDriver);
		trainRemovalListLock.unlock();
	}

	void TrainRemoval::removeTrainsBeforeNextFrameTick()
	{
		std::vector<TrainDriver*>::iterator itr=trainsToBeRemoved.begin();
		for(;itr!=trainsToBeRemoved.end();)
		{
			std::string trainLine = (*itr)->getTrainLine();
			int trainId = (*itr)->getTrainId();
			TrainDriver *nextDriver = (*itr)->getNextDriver();
			TrainController<Person_MT> *trainController = TrainController<Person_MT>::getInstance();
			std::vector <Role<Person_MT>*> activeTrains = trainController->getActiveTrainsForALine(trainLine);
			std::vector<Role<Person_MT>*>::iterator it = activeTrains.begin();
			for(;it!=activeTrains.end();it++)
			{
				TrainDriver *driver = dynamic_cast<TrainDriver*>(*it);
				if(driver->getNextDriver() == (*itr))
				{
					if(nextDriver)
					{
						driver->setNextDriver(nextDriver);
					}
					else
						driver->setNextDriver(nullptr);
				}
			}
			const TrainPlatformMover &pathMover = (*itr)->getMovement()->getTrainPlatformMover();
			const std::vector<Platform*>& prevPlatforms = pathMover.getPrevPlatforms();
			std::vector<Platform*>::const_iterator platfromItr;
			for (platfromItr = prevPlatforms.begin(); platfromItr != prevPlatforms.end(); platfromItr++)
			{
				Platform* prev = (*platfromItr);
				std::string stationNo = prev->getStationNo();
				Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
				TrainStationAgent *trainStationAgent = dynamic_cast<TrainStationAgent*>(stationAgent);
				std::map<std::string, TrainDriver*> lastTrainDriver=trainStationAgent->getLastDriver();
				std::map<std::string, TrainDriver*>::iterator ilastTrainDriver=lastTrainDriver.find(trainLine);
				if(ilastTrainDriver != lastTrainDriver.end())
				{
					if(ilastTrainDriver->second == (*itr))
					{
						lastTrainDriver[trainLine]=nullptr;
					}
				}
			}

			Platform *lastPlatform = (*itr)->getNextPlatform();
			if(lastPlatform!=nullptr)
			{
				std::string stationNo = lastPlatform->getStationNo();
				Agent* stationAgent = TrainController<Person_MT>::getAgentFromStation(stationNo);
				TrainStationAgent *trainStationAgent=dynamic_cast<TrainStationAgent*>(stationAgent);
				std::map<std::string, TrainDriver*> lastTrainDriver=trainStationAgent->getLastDriver();
				std::map<std::string, TrainDriver*>::iterator ilastTrainDriver=lastTrainDriver.find(trainLine);
				if(ilastTrainDriver != lastTrainDriver.end())
				{
					if(ilastTrainDriver->second == (*itr))
					{
						lastTrainDriver[trainLine]=nullptr;
					}
				}
			}

			trainController->removeFromListOfActiveTrainsInLine(trainLine,*itr);
			ServiceController::getInstance()->removeTrainIdAndTrainDriverInMap(trainId,trainLine,(*itr));
			trainsToBeRemoved.erase(itr);
			messaging::MessageBus::PostMessage(TrainController<Person_MT>::getInstance(),
										MSG_TRAIN_BACK_DEPOT, messaging::MessageBus::MessagePtr(new TrainMessage((*itr)->getParent())));
		}
	}
}
}

