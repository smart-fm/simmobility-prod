//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ActivityFacets.hpp"
#include "logging/Log.hpp"

sim_mob::ActivityPerformerBehavior::ActivityPerformerBehavior(sim_mob::Person* parentAgent, sim_mob::ActivityPerformer* parentRole, std::string roleName) :
	BehaviorFacet(parentAgent),
	parentActivity(parentRole)
{}



void sim_mob::ActivityPerformerBehavior::frame_init() {
	throw std::runtime_error("ActivityPerformerBehavior::frame_init() is not implemented yet");
}

void sim_mob::ActivityPerformerBehavior::frame_tick() {
	throw std::runtime_error("ActivityPerformerBehavior::frame_tick() is not implemented yet");
}

void sim_mob::ActivityPerformerBehavior::frame_tick_output() {
	throw std::runtime_error("ActivityPerformerBehavior::frame_tick_output() is not implemented yet");
}

void sim_mob::ActivityPerformerMovement::frame_init() {
	parentActivity->initializeRemainingTime();
}

void sim_mob::ActivityPerformerMovement::frame_tick() {
	parentActivity->updateRemainingTime();
	if(parentActivity->remainingTimeToComplete <= 0){
		getParent()->setToBeRemoved();
	}
	getParent()->setRemainingTimeThisTick(0.0);
}

void sim_mob::ActivityPerformerMovement::frame_tick_output() {
	ActivityPerformerUpdateParams &p = parentActivity->getParams();
	LogOut("(\"Activity\""
			<<","<<p.now.frame()
			<<","<<getParent()->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(getParent()->xPos)
			<<"\",\"yPos\":\""<<static_cast<int>(getParent()->yPos)
			<<"\"})"<<std::endl);
}

sim_mob::ActivityPerformerMovement::ActivityPerformerMovement(sim_mob::Person* parentAgent, sim_mob::ActivityPerformer* parentRole, std::string roleName):
	MovementFacet(parentAgent), parentActivity(parentRole) {
}

