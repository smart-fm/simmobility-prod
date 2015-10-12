//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ActivityFacets.hpp"
#include "logging/Log.hpp"
#include "geospatial/network/Node.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

sim_mob::ActivityPerformerBehavior::ActivityPerformerBehavior(sim_mob::Person* parentAgent) :
	BehaviorFacet(parentAgent), parentActivity(nullptr)
{}

sim_mob::ActivityPerformerBehavior::~ActivityPerformerBehavior()
{}

void sim_mob::ActivityPerformerBehavior::frame_init()
{
	throw std::runtime_error("ActivityPerformerBehavior::frame_init() is not implemented yet");
}

void sim_mob::ActivityPerformerBehavior::frame_tick()
{
	throw std::runtime_error("ActivityPerformerBehavior::frame_tick() is not implemented yet");
}

void sim_mob::ActivityPerformerBehavior::frame_tick_output()
{
	throw std::runtime_error("ActivityPerformerBehavior::frame_tick_output() is not implemented yet");
}

sim_mob::ActivityPerformerMovement::~ActivityPerformerMovement()
{
	/*if(travelMetric.started)
	{
		finalizeTravelTimeMetric();
	}*/
}
void sim_mob::ActivityPerformerMovement::frame_init()
{
	parentActivity->initializeRemainingTime();
	parentActivity->setTravelTime(parentActivity->getRemainingTimeToComplete());
	//startTravelTimeMetric();
}

void sim_mob::ActivityPerformerMovement::frame_tick()
{
	parentActivity->updateRemainingTime();
	if(parentActivity->getRemainingTimeToComplete() <= 0)
	{
		getParent()->setToBeRemoved();
	}
	getParent()->setRemainingTimeThisTick(0.0);
}

void sim_mob::ActivityPerformerMovement::frame_tick_output() {}

sim_mob::ActivityPerformerMovement::ActivityPerformerMovement(sim_mob::Person* parentAgent):
	MovementFacet(parentAgent), parentActivity(nullptr) {}

sim_mob::TravelMetric& sim_mob::ActivityPerformerMovement::startTravelTimeMetric()
{
	//travelMetric.started = true;
	return  travelMetric;
}
sim_mob::TravelMetric& sim_mob::ActivityPerformerMovement::finalizeTravelTimeMetric()
{
	//getParent()->serializeCBD_Activity(travelMetric);
	//travelMetric.finalized = true;
	return  travelMetric;
}

sim_mob::Conflux* sim_mob::ActivityPerformerMovement::getStartingConflux() const
{
	const Node* activityLocation = parentActivity->getLocation();
	if(activityLocation) //activity locations must ideally be multinodes
	{
		return ConfigManager::GetInstanceRW().FullConfig().getConfluxForNode(activityLocation);
	}
	return nullptr;
}
