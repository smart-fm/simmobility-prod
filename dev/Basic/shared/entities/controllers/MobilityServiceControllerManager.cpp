/*
 * MobilityServiceControllerManager.cpp
 *
 *  Created on: Apr 12, 2017
 *      Author: Akshay Padmanabha
 */

#include "MobilityServiceControllerManager.hpp"
#include "entities/controllers/GreedyController.hpp"
#include "entities/controllers/OnHailTaxiController.hpp"
#include "entities/controllers/SharedController.hpp"
#include "entities/controllers/IncrementalSharing.hpp"
#include "entities/controllers/ProximityBased.hpp"
#include "entities/controllers/AMOD_Controller.hpp"

using namespace sim_mob;

MobilityServiceControllerManager *MobilityServiceControllerManager::instance = nullptr;

MobilityServiceControllerManager *MobilityServiceControllerManager::GetInstance()
{
#ifndef NDEBUG
	instance->consistencyChecks();
#endif
	return instance;
}

MobilityServiceControllerManager::~MobilityServiceControllerManager()
{
}

bool MobilityServiceControllerManager::RegisterMobilityServiceControllerManager(const MutexStrategy &mtxStrat)
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
                                                                    unsigned int scheduleComputationPeriod,
                                                                    unsigned controllerId, std::string tripSupportMode,
                                                                    unsigned maxAggregatedRequests,bool studyAreaEnabledController,
                                                                    unsigned int toleratedExtraTime,unsigned int maxWaitingTime,bool parkingEnabled)
{

#ifndef NDEBUG
	sim_mob::consistencyChecks(type);
#endif

	TT_EstimateType ttEstimateType = EUCLIDEAN_ESTIMATION;
	MobilityServiceController *controller;
	switch (type)
	{
	case SERVICE_CONTROLLER_GREEDY:
	{
		controller = new GreedyController(getMutexStrategy(), scheduleComputationPeriod, controllerId, tripSupportMode,
		                                      ttEstimateType,maxAggregatedRequests,studyAreaEnabledController,toleratedExtraTime,maxWaitingTime,parkingEnabled);
		break;
	}
	case SERVICE_CONTROLLER_SHARED:
	{
		controller = new SharedController(getMutexStrategy(), scheduleComputationPeriod, controllerId, tripSupportMode, ttEstimateType,maxAggregatedRequests,studyAreaEnabledController,toleratedExtraTime,maxWaitingTime,parkingEnabled);
		break;
	}
	case SERVICE_CONTROLLER_FRAZZOLI:
	{
		controller = new FrazzoliController(getMutexStrategy(), scheduleComputationPeriod, controllerId, tripSupportMode,
		                                    ttEstimateType,maxAggregatedRequests,studyAreaEnabledController,toleratedExtraTime,maxWaitingTime,parkingEnabled);
		break;
	}
	case SERVICE_CONTROLLER_ON_HAIL:
	{
		controller = new OnHailTaxiController(getMutexStrategy(), controllerId,tripSupportMode,maxAggregatedRequests,studyAreaEnabledController,toleratedExtraTime,maxWaitingTime,parkingEnabled);
		break;
	}
	case SERVICE_CONTROLLER_INCREMENTAL:
	{
		controller = new IncrementalSharing(getMutexStrategy(), scheduleComputationPeriod, controllerId, tripSupportMode,
		                                    ttEstimateType,maxAggregatedRequests,studyAreaEnabledController,toleratedExtraTime,maxWaitingTime,parkingEnabled);
		break;
	}
	case SERVICE_CONTROLLER_PROXIMITY:
	{
		controller = new ProximityBased(getMutexStrategy(), scheduleComputationPeriod, controllerId, tripSupportMode,
		                                    ttEstimateType,maxAggregatedRequests,studyAreaEnabledController,toleratedExtraTime,maxWaitingTime,parkingEnabled);
		break;
	}
	case SERVICE_CONTROLLER_AMOD:
	{
		controller = new AMOD_Controller(getMutexStrategy(), scheduleComputationPeriod, controllerId, tripSupportMode,ttEstimateType,maxAggregatedRequests,studyAreaEnabledController,toleratedExtraTime,maxWaitingTime,parkingEnabled);
		break;
	}
	default:
	{
		return false;
	}
	}

	controllers.insert(controller);

#ifndef NDEBUG
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

const SvcControllerMap& MobilityServiceControllerManager::getControllers() const
{
	return controllers;
}

void MobilityServiceControllerManager::consistencyChecks() const
{
	const MobilityServiceController *controller;
	try
	{
		for (auto it = controllers.get<ctrlrId>().begin(); it != controllers.get<ctrlrId>().end(); ++it)
		{
			controller = *it;
			controller->consistencyChecks();
			sim_mob::consistencyChecks(controller->getServiceType());
		}
	}
	catch (const std::runtime_error &e)
	{
		std::stringstream msg;
		msg << "Error in mobilityControllerManager due to controller pointed by " <<
		    controller;
		msg << ". Search warn.log for this pointer to check if the controller has been removed. The error is "
		    << e.what();
		msg << " . The registered controllers are currently: " << getControllersStr();
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
	for (auto it = controllers.get<ctrlrId>().begin(); it != controllers.get<ctrlrId>().end(); ++it)
	{
		msg << "Pointer:" << *it << ":" << (*it)->toString() << ", ";
	}
	return msg.str();
}
