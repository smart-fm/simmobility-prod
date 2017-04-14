/*
 * VehicleControllerManager.cpp
 *
 *  Created on: Apr 12, 2017
 *      Author: Akshay Padmanabha
 */

#include "VehicleControllerManager.hpp"

#include "message/MessageBus.hpp"

namespace sim_mob
{
VehicleControllerManager* VehicleControllerManager::instance = nullptr;

VehicleControllerManager* VehicleControllerManager::GetInstance()
{
	return instance;
}

VehicleControllerManager::~VehicleControllerManager()
{
}

bool VehicleControllerManager::RegisterVehicleControllerManager(
	const MutexStrategy& mtxStrat)
{
	VehicleControllerManager *vehicleControllerManager
		= new VehicleControllerManager(mtxStrat);

	if (!instance)
	{
		instance = vehicleControllerManager;
		return true;
	}

	return false;
}

bool VehicleControllerManager::HasVehicleControllerManager()
{
	return (instance != nullptr);
}

bool VehicleControllerManager::addVehicleController(unsigned int id,
	VehicleController* vc)
{
	if (controllers.count(id) > 0)
	{
		return false;
	}

	controllers.insert(std::make_pair(id, vc));

	return true;
}

bool VehicleControllerManager::removeVehicleController(unsigned int id)
{
	std::map<unsigned int, VehicleController*>::iterator it
		= controllers.find(id);

	if (it != controllers.end())
	{
		controllers.erase(it);
		return true;
	}

	return false;
}

// TODO: Only add to the right controllers
void VehicleControllerManager::addVehicleDriver(Person* person)
{
	auto controller = controllers.begin();

	while (controller != controllers.end())
	{
		controller->second->addVehicleDriver(person);
		controller++;
	}
}

// TODO: Only remove from the right controllers
void VehicleControllerManager::removeVehicleDriver(Person* person)
{
	auto controller = controllers.begin();

	while (controller != controllers.end())
	{
		controller->second->removeVehicleDriver(person);
		controller++;
	}
}

Entity::UpdateStatus VehicleControllerManager::frame_init(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus VehicleControllerManager::frame_tick(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}

void VehicleControllerManager::frame_output(timeslice now)
{
}

std::map<unsigned int, VehicleController*> VehicleControllerManager::getControllers()
{
	return controllers;
}

bool VehicleControllerManager::isNonspatial()
{
	return true;
}
}


