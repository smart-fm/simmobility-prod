/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file WaitBusActivity.cpp
 *
 *  \author Yao Jin
 */

#include "WaitBusActivity.hpp"
#include "entities/Person.hpp"

using std::vector;
using namespace sim_mob;

sim_mob::WaitBusActivity::WaitBusActivity(Agent* parent, std::string roleName) :
		Role(parent,  roleName), params(parent->getGenerator()), remainingTimeToComplete(0),
		busStop(nullptr), passengerFlag(nullptr) {

}

sim_mob::WaitBusActivity::~WaitBusActivity() {

}

Role* sim_mob::WaitBusActivity::clone(Person* parent) const
{
	return new WaitBusActivity(parent);
}

void sim_mob::WaitBusActivity::frame_init(UpdateParams& p) {
	initializeRemainingTime();
}

void sim_mob::WaitBusActivity::frame_tick(UpdateParams& p) {
	updateRemainingTime();
	if(remainingTimeToComplete <= 0){
		parent->setToBeRemoved();
	}
	else
	{
//		std::cout << " This activity still has " << this->remainingTimeToComplete << " to complete\n";
	}
}

void sim_mob::WaitBusActivity::frame_tick_output(const UpdateParams& p) {

}

void sim_mob::WaitBusActivity::frame_tick_output_mpi(timeslice now) {

}

UpdateParams& sim_mob::WaitBusActivity::make_frame_tick_params(timeslice now) {
	params.reset(now);
	return params;
}

std::vector<sim_mob::BufferedBase*> sim_mob::WaitBusActivity::getSubscriptionParams() {
	vector<BufferedBase*> res;
	return res;
}

void sim_mob::WaitBusActivity::initializeRemainingTime()
{
	remainingTimeToComplete = activityEndTime.offsetMS_From(ConfigParams::GetInstance().simStartTime)
			- activityStartTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);
}

void sim_mob::WaitBusActivity::updateRemainingTime()
{
	remainingTimeToComplete = std::max(0, remainingTimeToComplete - int(ConfigParams::GetInstance().baseGranMS));
}
