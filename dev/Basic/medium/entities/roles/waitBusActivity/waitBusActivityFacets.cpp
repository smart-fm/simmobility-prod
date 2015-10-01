/*
 * WaitBusActivityFacets.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#include "waitBusActivityFacets.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "geospatial/BusStop.hpp"
#include "waitBusActivity.hpp"

namespace sim_mob
{

namespace medium
{

WaitBusActivityBehavior::WaitBusActivityBehavior() :
BehaviorFacet(), parentWaitBusActivity(nullptr)
{
}

WaitBusActivityBehavior::~WaitBusActivityBehavior()
{
}

WaitBusActivityMovement::WaitBusActivityMovement() :
MovementFacet(), parentWaitBusActivity(nullptr), totalTimeToCompleteMS(0)
{
}

WaitBusActivityMovement::~WaitBusActivityMovement()
{
}

void WaitBusActivityMovement::setParentWaitBusActivity(sim_mob::medium::WaitBusActivity* parentWaitBusActivity)
{
	this->parentWaitBusActivity = parentWaitBusActivity;
}

void WaitBusActivityBehavior::setParentWaitBusActivity(sim_mob::medium::WaitBusActivity* parentWaitBusActivity)
{
	this->parentWaitBusActivity = parentWaitBusActivity;
}

void WaitBusActivityMovement::frame_init()
{
	totalTimeToCompleteMS = 0;
	if (parentWaitBusActivity)
	{
		UpdateParams& params = parentWaitBusActivity->getParams();
		Person* person = parentWaitBusActivity->parent;
		person->setStartTime(params.now.ms());
	}
}

void WaitBusActivityMovement::frame_tick()
{
	unsigned int tickMS = ConfigManager::GetInstance().FullConfig().baseGranMS();
	totalTimeToCompleteMS += tickMS;
	if (parentWaitBusActivity)
	{
		parentWaitBusActivity->setWaitingTime(totalTimeToCompleteMS);
		parentWaitBusActivity->setTravelTime(totalTimeToCompleteMS);
	}
	parentWaitBusActivity->parent->setRemainingTimeThisTick(0);
}

void WaitBusActivityMovement::frame_tick_output()
{

}

sim_mob::Conflux* WaitBusActivityMovement::getStartingConflux() const
{
	const BusStopAgent* stopAg = sim_mob::medium::BusStopAgent::findBusStopAgentByBusStop(parentWaitBusActivity->getStop());
	if (stopAg)
	{
		return stopAg->getParentSegmentStats()->getRoadSegment()->getParentConflux();
	}
	return nullptr;
}

}

TravelMetric & medium::WaitBusActivityMovement::startTravelTimeMetric()
{
	return travelMetric;
}

TravelMetric & medium::WaitBusActivityMovement::finalizeTravelTimeMetric()
{
	return travelMetric;
}

} /* namespace sim_mob */


