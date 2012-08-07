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

sim_mob::ActivityPerformer::ActivityPerformer(Agent* parent) :
		Role(parent), params(parent->getGenerator()) {
	//NOTE: Be aware that a null parent is certainly possible; what if we want to make a "generic" Pedestrian?
	//      The RoleManger in particular relies on this. ~Seth
}

sim_mob::ActivityPerformer::ActivityPerformer(Agent* parent,
		const sim_mob::Activity& currActivity) :
		Role(parent), params(parent->getGenerator()){
	//NOTE: Be aware that a null parent is certainly possible; what if we want to make a "generic" Pedestrian?
	//      The RoleManger in particular relies on this. ~Seth
	activityStartTime = currActivity.startTime;
	activityEndTime = currActivity.endTime;
	location = currActivity.location;
}

Role* sim_mob::ActivityPerformer::clone(Person* parent) const
{
	return new ActivityPerformer(parent);
}

sim_mob::ActivityPerformerUpdateParams::ActivityPerformerUpdateParams(
		boost::mt19937& gen) :
		UpdateParams(gen) {
}

void sim_mob::ActivityPerformer::frame_init(UpdateParams& p) {
	this->initializeRemainingTime();
}

void sim_mob::ActivityPerformer::frame_tick(UpdateParams& p) {
	this->updateRemainingTime();
	if(this->remainingTimeToComplete == 0){
		parent->setToBeRemoved();
	}
}

void sim_mob::ActivityPerformer::frame_tick_output(const UpdateParams& p) {
}

void sim_mob::ActivityPerformer::frame_tick_output_mpi(frame_t frameNumber) {
}

UpdateParams& sim_mob::ActivityPerformer::make_frame_tick_params(
		frame_t frameNumber, unsigned int currTimeMS) {
	params.reset(frameNumber, currTimeMS);
	return params;
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
	this->activityEndTime = activityEndTime;
}

sim_mob::DailyTime sim_mob::ActivityPerformer::getActivityStartTime() const {
	return activityStartTime;
}

void sim_mob::ActivityPerformer::setActivityStartTime(
		sim_mob::DailyTime activityStartTime) {
	this->activityStartTime = activityStartTime;
}

sim_mob::Node* sim_mob::ActivityPerformer::getLocation() const {
	return location;
}

void sim_mob::ActivityPerformer::setLocation(sim_mob::Node* location) {
	this->location = location;
}

void sim_mob::ActivityPerformer::initializeRemainingTime() {
	this->remainingTimeToComplete = this->activityEndTime.offsetMS_From(
			ConfigParams::GetInstance().simStartTime)
			- this->activityStartTime.offsetMS_From(
					ConfigParams::GetInstance().simStartTime);
}

void sim_mob::ActivityPerformer::updateRemainingTime() {
	this->remainingTimeToComplete =
			((this->remainingTimeToComplete - ConfigParams::GetInstance().baseGranMS) >=0)?
					(this->remainingTimeToComplete - ConfigParams::GetInstance().baseGranMS):
					0;

}

