/*
 * WaitTaxiActivityFacets.cpp
 *
 *  Created on: Nov 28, 2016
 *      Author: zhang huai peng
 */

#include "WaitTaxiActivityFacets.hpp"
#include "WaitTaxiActivity.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
namespace sim_mob
{
namespace medium
{
WaitTaxiActivityBehavior::WaitTaxiActivityBehavior() :
BehaviorFacet(), waitTaxiActivity(nullptr)
{
}

WaitTaxiActivityBehavior::~WaitTaxiActivityBehavior()
{
}

WaitTaxiActivityMovement::WaitTaxiActivityMovement() :
MovementFacet(), waitTaxiActivity(nullptr)
{
}

WaitTaxiActivityMovement::~WaitTaxiActivityMovement()
{
}

void WaitTaxiActivityMovement::setWaitTaxiActivity(WaitTaxiActivity* activity)
{
	waitTaxiActivity = activity;
}

void WaitTaxiActivityBehavior::setWaitTaxiActivity(WaitTaxiActivity* activity)
{
	waitTaxiActivity = activity;
}

void WaitTaxiActivityMovement::frame_init()
{
	if(waitTaxiActivity)
	{
		UpdateParams& params = waitTaxiActivity->getParams();
		Person* person = waitTaxiActivity->parent;
		person->setStartTime(params.now.ms());
	}
}

void WaitTaxiActivityMovement::frame_tick()
{
	unsigned int tickMS = ConfigManager::GetInstance().FullConfig().baseGranMS();
	if(waitTaxiActivity)
	{
		waitTaxiActivity->increaseWaitingTime(tickMS);
		waitTaxiActivity->setTravelTime(waitTaxiActivity->getWaitingTime());
	}
	waitTaxiActivity->parent->setRemainingTimeThisTick(0);
}

std::string WaitTaxiActivityMovement::frame_tick_output()
{
	return std::string();
}

Conflux* WaitTaxiActivityMovement::getStartingConflux() const
{
	return nullptr;
}

TravelMetric& WaitTaxiActivityMovement::startTravelTimeMetric()
{
	return travelMetric;
}

TravelMetric& WaitTaxiActivityMovement::finalizeTravelTimeMetric()
{
	return travelMetric;
}
}
}
