/*
 * WaitBusActivityFacets.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#include "WaitBusActivityFacets.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "geospatial/BusStop.hpp"
#include "WaitBusActivity.hpp"

namespace sim_mob {
namespace medium {
WaitBusActivityBehavior::WaitBusActivityBehavior(sim_mob::Person* parentAgent) :
		BehaviorFacet(parentAgent), parentWaitBusActivity(nullptr)
{}

WaitBusActivityBehavior::~WaitBusActivityBehavior()
{}

void sim_mob::medium::WaitBusActivityBehavior::frame_init()
{}

void sim_mob::medium::WaitBusActivityBehavior::frame_tick()
{}

void sim_mob::medium::WaitBusActivityBehavior::frame_tick_output()
{}

WaitBusActivityMovement::WaitBusActivityMovement(sim_mob::Person* parentAgent) :
		MovementFacet(parentAgent), parentWaitBusActivity(nullptr), totalTimeToCompleteMS(0)
{}

WaitBusActivityMovement::~WaitBusActivityMovement()
{}

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
	if(parentWaitBusActivity)
	{
		UpdateParams& params = parentWaitBusActivity->getParams();
		Person* person = parentWaitBusActivity->getParent();
		person->setStartTime(params.now.ms());
	}
}

void WaitBusActivityMovement::frame_tick()
{
	unsigned int tickMS = ConfigManager::GetInstance().FullConfig().baseGranMS();
	totalTimeToCompleteMS += tickMS;
	if(parentWaitBusActivity)
	{
		parentWaitBusActivity->setWaitingTime(totalTimeToCompleteMS);
		parentWaitBusActivity->setTravelTime(totalTimeToCompleteMS);
	}
	parent->setRemainingTimeThisTick(0);
}

void WaitBusActivityMovement::frame_tick_output()
{}

sim_mob::Conflux* WaitBusActivityMovement::getStartingConflux() const
{
	const sim_mob::medium::BusStopAgent* stopAg = sim_mob::medium::BusStopAgent::findBusStopAgentByBusStop(parentWaitBusActivity->getStop());
	if(stopAg)
	{
		return stopAg->getParentSegmentStats()->getRoadSegment()->getParentConflux();
	}
	return nullptr;
}

TravelMetric& WaitBusActivityMovement::startTravelTimeMetric()
{
	return travelMetric;
}

TravelMetric& WaitBusActivityMovement::finalizeTravelTimeMetric()
{
	return travelMetric;
}

} /* namespace medium */
} /* namespace sim_mob */
