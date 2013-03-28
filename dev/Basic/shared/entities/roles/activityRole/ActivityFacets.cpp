#include "ActivityFacets.hpp"
#include "entities/UpdateParams.hpp"

sim_mob::ActivityPerformerBehavior::ActivityPerformerBehavior(sim_mob::Agent* parentAgent, sim_mob::ActivityPerformer* parentRole, std::string roleName) :
BehaviorFacet(parentAgent, roleName), location(nullptr), remainingTimeToComplete(0), parentActivity(parentRole)  {}


void sim_mob::ActivityPerformerBehavior::frame_init(UpdateParams& p) {
	this->initializeRemainingTime();
}

void sim_mob::ActivityPerformerBehavior::frame_tick(UpdateParams& p) {
	this->updateRemainingTime();
	if(this->remainingTimeToComplete <= 0){
		parentAgent->setToBeRemoved();
	}
}

void sim_mob::ActivityPerformerBehavior::frame_tick_output(const UpdateParams& p) {
	LogOut("(\"Activity\""
			<<","<<p.now.frame()
			<<","<<parentAgent->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(parentAgent->xPos)
			<<"\",\"yPos\":\""<<static_cast<int>(parentAgent->yPos)
			<<"\"})"<<std::endl);
}

sim_mob::UpdateParams& sim_mob::ActivityPerformerBehavior::make_frame_tick_params(timeslice now) {
	parentActivity->params.reset(now);
	return parentActivity->params;
}

void sim_mob::ActivityPerformerBehavior::frame_tick_output_mpi(timeslice now) {
}

sim_mob::DailyTime sim_mob::ActivityPerformerBehavior::getActivityEndTime() const {
	return activityEndTime;
}

void sim_mob::ActivityPerformerBehavior::setActivityEndTime(
		sim_mob::DailyTime activityEndTime) {
	std::cout << "activityEndTime = " << activityEndTime.toString() <<"\n";
	this->activityEndTime = activityEndTime;
}

sim_mob::DailyTime sim_mob::ActivityPerformerBehavior::getActivityStartTime() const {
	return activityStartTime;
}

void sim_mob::ActivityPerformerBehavior::setActivityStartTime(
		sim_mob::DailyTime activityStartTime) {
	std::cout << "activityStartTime = " << activityStartTime.toString() <<"\n";
	this->activityStartTime = DailyTime(activityStartTime.toString());
}

sim_mob::Node* sim_mob::ActivityPerformerBehavior::getLocation() const {
	return location;
}

void sim_mob::ActivityPerformerBehavior::setLocation(sim_mob::Node* location) {
	this->location = location;
}

void sim_mob::ActivityPerformerBehavior::initializeRemainingTime() {
	this->remainingTimeToComplete =
			this->activityEndTime.offsetMS_From(ConfigParams::GetInstance().simStartTime)
			- this->activityStartTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);
}

void sim_mob::ActivityPerformerBehavior::updateRemainingTime() {
	this->remainingTimeToComplete = std::max(0, this->remainingTimeToComplete - int(ConfigParams::GetInstance().baseGranMS));
}

void sim_mob::ActivityPerformerMovement::frame_init(UpdateParams& p) {
	throw std::runtime_error("ActivityPerformerMovement::frame_init() shouldn't have been called. Activity works only with behavior facet.");
}

void sim_mob::ActivityPerformerMovement::frame_tick(UpdateParams& p) {
	throw std::runtime_error("ActivityPerformerMovement::frame_tick() shouldn't have been called. Activity works only with behavior facet.");
}

void sim_mob::ActivityPerformerMovement::frame_tick_output(const UpdateParams& p) {
	throw std::runtime_error("ActivityPerformerMovement::frame_tick_output() shouldn't have been called. Activity works only with behavior facet.");
}

void sim_mob::ActivityPerformerMovement::frame_tick_output_mpi(timeslice now) {
	throw std::runtime_error("ActivityPerformerMovement::frame_tick_output_mpi() shouldn't have been called. Activity works only with behavior facet.");
}

UpdateParams& sim_mob::ActivityPerformerMovement::make_frame_tick_params(timeslice now) {
	throw std::runtime_error("ActivityPerformerMovement::make_frame_tick_params() shouldn't have been called. Activity works only with behavior facet.");
}

sim_mob::ActivityPerformerMovement::ActivityPerformerMovement(
		sim_mob::Agent* parentAgent, sim_mob::Role* parentRole,
		std::string roleName) {
}

std::vector<sim_mob::BufferedBase*> sim_mob::ActivityPerformerMovement::getSubscriptionParams() {
	throw std::runtime_error("ActivityPerformerMovement::getSubscriptionParams() shouldn't have been called. Activity works only with behavior facet.");
}





