/*
 * MobilityServiceControllerManager.cpp
 *
 *  Created on: Apr 12, 2017
 *      Author: Akshay Padmanabha
 */

#include "MobilityServiceControllerManager.hpp"
#include "entities/controllers/GreedyTaxiController.hpp"
#include "entities/controllers/OnHailTaxiController.hpp"
#include "entities/controllers/SharedTaxiController.hpp"

#include "message/MessageBus.hpp"

namespace sim_mob
{
MobilityServiceControllerManager* MobilityServiceControllerManager::instance = nullptr;

MobilityServiceControllerManager* MobilityServiceControllerManager::GetInstance()
{
	return instance;
}

MobilityServiceControllerManager::~MobilityServiceControllerManager()
{
}

bool MobilityServiceControllerManager::RegisterMobilityServiceControllerManager(
	const MutexStrategy& mtxStrat)
{
	MobilityServiceControllerManager *mobilityServiceControllerManager
		= new MobilityServiceControllerManager(mtxStrat);

	if (!instance)
	{
		instance = mobilityServiceControllerManager;
		return true;
	}

	return false;
}

bool MobilityServiceControllerManager::HasMobilityServiceControllerManager()
{
	return (instance != nullptr);
}

bool MobilityServiceControllerManager::addMobilityServiceController(unsigned int id, unsigned int type, unsigned int scheduleComputationPeriod)
{
	if (controllers.count(id) > 0)
	{
		return false;
	}

    if (type == 1)
    {
        GreedyTaxiController* svc = new GreedyTaxiController(getMutexStrategy(), scheduleComputationPeriod);
        controllers.insert(std::make_pair(id, svc));
    }
    else if (type == 2)
    {
        SharedTaxiController* svc = new SharedTaxiController(getMutexStrategy(), scheduleComputationPeriod);
        controllers.insert(std::make_pair(id, svc));
    }
    else if (type == 3)
    {
        OnHailTaxiController* svc = new OnHailTaxiController(getMutexStrategy(), scheduleComputationPeriod);
        controllers.insert(std::make_pair(id, svc));
    }
    else
    {
    	return false;
    }

	return true;
}

bool MobilityServiceControllerManager::removeMobilityServiceController(unsigned int id)
{
	std::map<unsigned int, MobilityServiceController*>::iterator it
		= controllers.find(id);

	if (it != controllers.end())
	{
		controllers.erase(it);
		return true;
	}

	return false;
}

Entity::UpdateStatus MobilityServiceControllerManager::frame_init(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus MobilityServiceControllerManager::frame_tick(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}

void MobilityServiceControllerManager::frame_output(timeslice now)
{
}

std::map<unsigned int, MobilityServiceController*> MobilityServiceControllerManager::getControllers()
{
	return controllers;
}

bool MobilityServiceControllerManager::isNonspatial()
{
	return true;
}
}
