//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ActivityFacets.hpp"
#include "logging/Log.hpp"

sim_mob::ActivityPerformerBehavior::ActivityPerformerBehavior(sim_mob::Person* parentAgent, sim_mob::ActivityPerformer* parentRole, std::string roleName) :
	BehaviorFacet(parentAgent),
	parentActivity(parentRole)
{}


void sim_mob::ActivityPerformerBehavior::frame_init(UpdateParams& p) {
	parentActivity->initializeRemainingTime();
}

void sim_mob::ActivityPerformerBehavior::frame_tick(UpdateParams& p) {
	parentActivity->updateRemainingTime();
	if(parentActivity->remainingTimeToComplete <= 0){
		getParent()->setToBeRemoved();
	}
}

void sim_mob::ActivityPerformerBehavior::frame_tick_output(const UpdateParams& p) {
	LogOut("(\"Activity\""
			<<","<<p.now.frame()
			<<","<<getParent()->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(getParent()->xPos)
			<<"\",\"yPos\":\""<<static_cast<int>(getParent()->yPos)
			<<"\"})"<<std::endl);
}

void sim_mob::ActivityPerformerMovement::frame_init(UpdateParams& p) {
	//parentActivity->initializeRemainingTime();
	//throw std::runtime_error("ActivityPerformerMovement::frame_init() shouldn't have been called. Activity works only with behavior facet.");
}

void sim_mob::ActivityPerformerMovement::frame_tick(UpdateParams& p) {
	/*parentActivity->updateRemainingTime();
	if(parentActivity->remainingTimeToComplete <= 0){
		parentAgent->setToBeRemoved();
	}*/
	//throw std::runtime_error("ActivityPerformerMovement::frame_tick() shouldn't have been called. Activity works only with behavior facet.");
}

void sim_mob::ActivityPerformerMovement::frame_tick_output(const UpdateParams& p) {
	//throw std::runtime_error("ActivityPerformerMovement::frame_tick_output() shouldn't have been called. Activity works only with behavior facet.");
}

sim_mob::ActivityPerformerMovement::ActivityPerformerMovement(sim_mob::Person* parentAgent, sim_mob::ActivityPerformer* parentRole, std::string roleName):
	MovementFacet(parentAgent), parentActivity(parentRole) {
}

