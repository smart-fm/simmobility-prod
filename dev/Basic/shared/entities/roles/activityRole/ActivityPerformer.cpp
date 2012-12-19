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

sim_mob::ActivityPerformer::ActivityPerformer(Agent* parent, std::string roleName) :
		Role(parent,  roleName), params(parent->getGenerator()) {
	//NOTE: Be aware that a null parent is certainly possible; what if we want to make a "generic" Pedestrian?
	//      The RoleManger in particular relies on this. ~Seth
}

sim_mob::ActivityPerformer::ActivityPerformer(Agent* parent, const sim_mob::Activity& currActivity, std::string roleName) :
		Role(parent, roleName), params(parent->getGenerator()){
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
	if(this->remainingTimeToComplete <= 0){
		parent->setToBeRemoved();
	}
	else
	{
//		std::cout << " This activity still has " << this->remainingTimeToComplete << " to complete\n";
	}
}

void sim_mob::ActivityPerformer::frame_tick_output(const UpdateParams& p) {
#ifndef SIMMOB_DISABLE_OUTPUT
	LogOut("(\"Activity\""
			<<","<<p.now.frame()
			<<","<<parent->getId()
			<<",{"
			<<"\"xPos\":\""<<static_cast<int>(parent->xPos)
			<<"\",\"yPos\":\""<<static_cast<int>(parent->yPos)
			<<"\"})"<<std::endl);
#endif
}

void sim_mob::ActivityPerformer::frame_tick_output_mpi(timeslice now) {
}

UpdateParams& sim_mob::ActivityPerformer::make_frame_tick_params(timeslice now) {
	params.reset(now);
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
			-
			this->activityStartTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);
}

void sim_mob::ActivityPerformer::updateRemainingTime() {
	this->remainingTimeToComplete = std::max(0, this->remainingTimeToComplete - int(ConfigParams::GetInstance().baseGranMS));
}

