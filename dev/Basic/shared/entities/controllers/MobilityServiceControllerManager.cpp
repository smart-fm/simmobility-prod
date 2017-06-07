/*
 * MobilityServiceControllerManager.cpp
 *
 *  Created on: Apr 12, 2017
 *      Author: Akshay Padmanabha
 */

#include "MobilityServiceControllerManager.hpp"
#include "entities/controllers/GreedyTaxiController.hpp"
#include "entities/controllers/OnHailTaxiController.hpp"
#include "entities/controllers/SharedController.hpp"

using namespace sim_mob;

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

    switch(type)
	{
		case SERVICE_CONTROLLER_GREEDY:
		{
			GreedyTaxiController *controller = new GreedyTaxiController(getMutexStrategy(), scheduleComputationPeriod);
			controllers.insert(std::make_pair(id, controller));
			break;
		}
		case SERVICE_CONTROLLER_SHARED:
		{
			SharedController *controller = new SharedController(getMutexStrategy(), scheduleComputationPeriod);
			controllers.insert(std::make_pair(id, controller));
			break;
		}
		case SERVICE_CONTROLLER_ON_HAIL:
		{
			OnHailTaxiController *controller = new OnHailTaxiController(getMutexStrategy(), scheduleComputationPeriod);
			controllers.insert(std::make_pair(id, controller));
			break;
		}
		default:
			return false;
	}

	return true;
}

bool MobilityServiceControllerManager::removeMobilityServiceController(unsigned int id)
{
	std::map<unsigned int, MobilityServiceController*>::iterator it = controllers.find(id);

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

