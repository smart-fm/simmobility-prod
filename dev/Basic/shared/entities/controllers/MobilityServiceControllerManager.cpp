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
#include "logging/ControllerLog.hpp"
#include <stdexcept>

using namespace sim_mob;

MobilityServiceControllerManager* MobilityServiceControllerManager::instance = nullptr;

MobilityServiceControllerManager* MobilityServiceControllerManager::GetInstance()
{
#ifndef NDEBUG
	instance->consistencyChecks();
#endif
	return instance;
}

MobilityServiceControllerManager::~MobilityServiceControllerManager()
{
}

bool MobilityServiceControllerManager::RegisterMobilityServiceControllerManager(const MutexStrategy& mtxStrat)
{
	bool retValue;
	if (!instance)
	{
		instance = new MobilityServiceControllerManager(mtxStrat);
		retValue = true;
	}

	retValue = false;

#ifndef NDEBUG
	instance->consistencyChecks();
#endif

	return retValue;
}

bool MobilityServiceControllerManager::HasMobilityServiceControllerManager()
{
	bool retValue = (instance != nullptr);
#ifndef NDEBUG
	if (retValue)
		instance->consistencyChecks();
#endif
	return retValue;
}

bool MobilityServiceControllerManager::addMobilityServiceController(MobilityServiceControllerType type,
									unsigned int scheduleComputationPeriod, unsigned controllerId)
{

#ifndef NDEBUG
	sim_mob::consistencyChecks(type);
#endif

	TT_EstimateType ttEstimateType = EUCLIDEAN_ESTIMATION;
	MobilityServiceController* controller;
	switch(type)
	{
		case SERVICE_CONTROLLER_GREEDY:
		{
			controller = new GreedyTaxiController(getMutexStrategy(), scheduleComputationPeriod, controllerId, ttEstimateType);
			break;
		}
		case SERVICE_CONTROLLER_SHARED:
		{
			controller = new SharedController(getMutexStrategy(), scheduleComputationPeriod, controllerId, ttEstimateType) ;
			break;
		}
		case SERVICE_CONTROLLER_FRAZZOLI:
		{
			controller = new FrazzoliController(getMutexStrategy(), scheduleComputationPeriod, controllerId, ttEstimateType);
			break;
		}
		case SERVICE_CONTROLLER_ON_HAIL:
		{
			controller = new OnHailTaxiController(getMutexStrategy(), controllerId);
			break;
		}
		default:
		{
			return false;
		}
	}
	controllers.insert(std::make_pair(type, controller));

#ifndef NDEBUG
	controller->consistencyChecks();
	consistencyChecks();
#endif

	return true;
}

Entity::UpdateStatus MobilityServiceControllerManager::frame_init(timeslice now)
{
#ifndef NDEBUG
	consistencyChecks();
#endif
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus MobilityServiceControllerManager::frame_tick(timeslice now)
{
#ifndef NDEBUG
	consistencyChecks();
#endif
	return Entity::UpdateStatus::Continue;
}

void MobilityServiceControllerManager::frame_output(timeslice now)
{
#ifndef NDEBUG
	consistencyChecks();
#endif

}

const std::multimap<MobilityServiceControllerType, MobilityServiceController *>& MobilityServiceControllerManager::getControllers() const
{

	return controllers;
}

void MobilityServiceControllerManager::consistencyChecks() const
{
	const MobilityServiceController* controller;
	try{
		for (const std::pair<MobilityServiceControllerType, const MobilityServiceController*> p : controllers)
		{
			controller = p.second;
			controller->consistencyChecks();
			const MobilityServiceControllerType typeKey = p.first;
			sim_mob::consistencyChecks(typeKey);
			const MobilityServiceControllerType ownedType = controller->getServiceType();
			if (ownedType != typeKey)
			{
				std::stringstream msg; msg<<"A controller of type "<<ownedType<<" is associated to type "<<typeKey;
				throw std::runtime_error(msg.str());
			}
		}
	}catch(const std::runtime_error& e)
	{
		std::stringstream msg; msg<<"Error in mobilityControllerManager due to controller pointed by "<<
			controller;
		msg<<". Search warn.log for this pointer to check if the controller has been removed. The error is "<<e.what();
		msg<< " . The registered controllers are currently: "<<
				getControllersStr();
		throw std::runtime_error(msg.str());
	}
}


bool MobilityServiceControllerManager::isNonspatial()
{
#ifndef NDEBUG
	consistencyChecks();
#endif
	// A controller is not located in any specific place
	return true;
}

const std::string MobilityServiceControllerManager::getControllersStr() const
{
	std::stringstream msg;
	for (const std::pair<MobilityServiceControllerType, const MobilityServiceController*>& p: controllers)
	{
		msg<< "pointer:"<<p.second<<":"<< p.second->toString() <<", ";
	}
	return msg.
			str();
}
