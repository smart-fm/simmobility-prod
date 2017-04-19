/*
 * MobilityServiceControllerManager.cpp
 *
 *  Created on: Apr 12, 2017
 *      Author: Akshay Padmanabha
 */

#include "MobilityServiceControllerManager.hpp"

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

bool MobilityServiceControllerManager::addMobilityServiceController(unsigned int id,
	MobilityServiceController* controller)
{
	if (controllers.count(id) > 0)
	{
		return false;
	}

	controllers.insert(std::make_pair(id, controller));

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

// TODO: Only add to the right controllers
void MobilityServiceControllerManager::addVehicleDriver(Person* person)
{
	auto controller = controllers.begin();

	while (controller != controllers.end())
	{
		controller->second->addVehicleDriver(person);
		controller++;
	}
}

// TODO: Only remove from the right controllers
void MobilityServiceControllerManager::removeVehicleDriver(Person* person)
{
	auto controller = controllers.begin();

	while (controller != controllers.end())
	{
		controller->second->removeVehicleDriver(person);
		controller++;
	}
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



