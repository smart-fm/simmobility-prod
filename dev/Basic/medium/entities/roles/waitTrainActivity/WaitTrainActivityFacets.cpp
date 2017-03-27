/*
 * WaitTrainActivityFacets.cpp
 *
 *  Created on: Mar 23, 2016
 *      Author: zhang huai peng
 */

#include "WaitTrainActivityFacets.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "WaitTrainActivity.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

WaitTrainActivityBehavior::WaitTrainActivityBehavior() :
BehaviorFacet(), parentWaitTrainActivity(nullptr)
{
}

WaitTrainActivityBehavior::~WaitTrainActivityBehavior()
{
}

std::string WaitTrainActivityBehavior::frame_tick_output()
{
	return std::string();
}

WaitTrainActivityMovement::WaitTrainActivityMovement() :
MovementFacet(), parentWaitTrainActivity(nullptr)
{
}

WaitTrainActivityMovement::~WaitTrainActivityMovement()
{
}

void WaitTrainActivityMovement::setParent(sim_mob::medium::WaitTrainActivity* parentWaitTrainActivity)
{
	this->parentWaitTrainActivity = parentWaitTrainActivity;
}

void WaitTrainActivityBehavior::setParent(sim_mob::medium::WaitTrainActivity* parentWaitTrainActivity)
{
	this->parentWaitTrainActivity = parentWaitTrainActivity;
}

void WaitTrainActivityMovement::frame_init()
{
	if(parentWaitTrainActivity)
	{
		UpdateParams& params = parentWaitTrainActivity->getParams();
		Person* person = parentWaitTrainActivity->parent;
		person->setStartTime(params.now.ms());
	}
}

void WaitTrainActivityMovement::frame_tick()
{
	unsigned int tickMS = ConfigManager::GetInstance().FullConfig().baseGranMS();
	if(parentWaitTrainActivity)
	{
		parentWaitTrainActivity->increaseWaitingTime(tickMS);
		parentWaitTrainActivity->setTravelTime(parentWaitTrainActivity->getWaitingTime());
	}
	parentWaitTrainActivity->parent->setRemainingTimeThisTick(0);
}

std::string WaitTrainActivityMovement::frame_tick_output()
{
	return std::string();
}

TravelMetric& WaitTrainActivityMovement::startTravelTimeMetric()
{
	return travelMetric;
}

TravelMetric& WaitTrainActivityMovement::finalizeTravelTimeMetric()
{
	return travelMetric;
}
