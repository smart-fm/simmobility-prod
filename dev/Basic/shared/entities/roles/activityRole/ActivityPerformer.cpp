/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file Pedestrian.cpp
 *
 *  \author Harish
 */

#include "ActivityPerformer.hpp"
#include "entities/Person.hpp"
#include "geospatial/Node.hpp"
#include "util/OutputUtil.hpp"

using std::vector;
using namespace sim_mob;

sim_mob::ActivityPerformer::ActivityPerformer(sim_mob::Agent* parent, sim_mob::ActivityPerformerBehavior* behavior, sim_mob::ActivityPerformerMovement* movement, std::string roleName):
		Role(behavior, movement, parent, roleName), params(parent->getGenerator()), remainingTimeToComplete(0), location(nullptr) {
	//NOTE: Be aware that a null parent is certainly possible; what if we want to make a "generic" Pedestrian?
	//      The RoleManger in particular relies on this. ~Seth
}

sim_mob::ActivityPerformer::ActivityPerformer(Agent* parent, const sim_mob::Activity& currActivity, sim_mob::ActivityPerformerBehavior* behavior, sim_mob::ActivityPerformerMovement* movement, std::string roleName) :
		Role(behavior, movement, parent, roleName), params(parent->getGenerator()), remainingTimeToComplete(0), location(nullptr) {
	//NOTE: Be aware that a null parent is certainly possible; what if we want to make a "generic" Pedestrian?
	//      The RoleManger in particular relies on this. ~Seth
	activityStartTime = currActivity.startTime;
	activityEndTime = currActivity.endTime;
	location = currActivity.location;
}

//xuyan: Error, Do not what to do, comment out
Role* sim_mob::ActivityPerformer::clone(Person* parent) const
{

	ActivityPerformerBehavior* behavior = new ActivityPerformerBehavior(parent);
	ActivityPerformerMovement* movement = new ActivityPerformerMovement(parent);
	ActivityPerformer* activityRole = new ActivityPerformer(parent, behavior, movement, "activityRole");
	return activityRole;
}

sim_mob::ActivityPerformerUpdateParams::ActivityPerformerUpdateParams( boost::mt19937& gen) : UpdateParams(gen), skipThisFrame(true) {
}

std::vector<sim_mob::BufferedBase*> sim_mob::ActivityPerformer::getSubscriptionParams() {
	vector<BufferedBase*> res;
	return res;
}

sim_mob::DailyTime sim_mob::ActivityPerformer::getActivityEndTime() const {
	return activityEndTime;
}

void sim_mob::ActivityPerformer::setActivityEndTime(
		sim_mob::DailyTime activityEndTime) {
	std::cout << "activityEndTime = " << activityEndTime.toString() <<"\n";
	this->activityEndTime = activityEndTime;
}

sim_mob::DailyTime sim_mob::ActivityPerformer::getActivityStartTime() const {
	return activityStartTime;
}

void sim_mob::ActivityPerformer::setActivityStartTime(
		sim_mob::DailyTime activityStartTime) {
	std::cout << "activityStartTime = " << activityStartTime.toString() <<"\n";
	this->activityStartTime = DailyTime(activityStartTime.toString());
}

sim_mob::Node* sim_mob::ActivityPerformer::getLocation() const {
	return location;
}

void sim_mob::ActivityPerformer::setLocation(sim_mob::Node* location) {
	this->location = location;
}

void sim_mob::ActivityPerformer::initializeRemainingTime() {
	this->remainingTimeToComplete =
			this->activityEndTime.offsetMS_From(ConfigParams::GetInstance().simStartTime)
			- this->activityStartTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);
}

void sim_mob::ActivityPerformer::frame_tick_output_mpi(timeslice now) {
}

UpdateParams& sim_mob::ActivityPerformer::make_frame_tick_params(timeslice now) {
	params.reset(now);
	return params;
}

void sim_mob::ActivityPerformer::updateRemainingTime() {
	this->remainingTimeToComplete = std::max(0, this->remainingTimeToComplete - int(ConfigParams::GetInstance().baseGranMS));
}
