/*
 * WaitBusActivityFacets.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#include "waitBusActivityFacets.hpp"
#include "waitBusActivity.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/BusStop.hpp"

namespace sim_mob {

namespace medium {

WaitBusActivityBehavior::WaitBusActivityBehavior(sim_mob::Person* parentAgent) :
		BehaviorFacet(parentAgent), parentWaitBusActivity(nullptr)
{

}

WaitBusActivityBehavior::~WaitBusActivityBehavior()
{

}

WaitBusActivityMovement::WaitBusActivityMovement(sim_mob::Person* parentAgent):
		MovementFacet(parentAgent), parentWaitBusActivity(nullptr), totalTimeToCompleteMS(0)
{

}

WaitBusActivityMovement::~WaitBusActivityMovement()
{

}

void WaitBusActivityMovement::setParentWaitBusActivity(sim_mob::medium::WaitBusActivity* parentWaitBusActivity){
	this->parentWaitBusActivity = parentWaitBusActivity;
}

void WaitBusActivityBehavior::setParentWaitBusActivity(sim_mob::medium::WaitBusActivity* parentWaitBusActivity){
	this->parentWaitBusActivity = parentWaitBusActivity;
}

void WaitBusActivityMovement::frame_init(){
	totalTimeToCompleteMS = 0;
	if(parentWaitBusActivity){
		UpdateParams& params = parentWaitBusActivity->getParams();
		Person* person = parentWaitBusActivity->getParent();
		person->setStartTime(params.now.ms());
	}
}

void WaitBusActivityMovement::frame_tick()
{
	unsigned int tickMS = ConfigManager::GetInstance().FullConfig().baseGranMS();
	totalTimeToCompleteMS += tickMS;
	if(parentWaitBusActivity){
		parentWaitBusActivity->setWaitingTime(totalTimeToCompleteMS);
	}
	parent->setRemainingTimeThisTick(0);
}

void WaitBusActivityMovement::frame_tick_output(){

}

}
TravelMetric & medium::WaitBusActivityMovement::startTravelTimeMetric()
{
	return travelTimeMetric;
}

TravelMetric & medium::WaitBusActivityMovement::finalizeTravelTimeMetric()
{
	return travelTimeMetric;
}

} /* namespace sim_mob */
