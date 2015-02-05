//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "waitBusActivity.hpp"
#include "waitBusActivityFacets.hpp"
#include "entities/Person.hpp"
#include "geospatial/BusStop.hpp"
#include "entities/roles/driver/BusDriver.hpp"

using std::vector;
using namespace sim_mob;

namespace sim_mob {

namespace medium {

sim_mob::medium::WaitBusActivity::WaitBusActivity(Person* parent,
		MutexStrategy mtxStrat,
		sim_mob::medium::WaitBusActivityBehavior* behavior,
		sim_mob::medium::WaitBusActivityMovement* movement,
		std::string roleName, Role::type roleType) :
		sim_mob::Role(behavior, movement, parent, roleName, roleType),
		waitingTime(0), stop(nullptr), boardBus(false), failedBoardingTimes(0)
{}

Role* sim_mob::medium::WaitBusActivity::clone(Person* parent) const {
	WaitBusActivityBehavior* behavior = new WaitBusActivityBehavior(parent);
	WaitBusActivityMovement* movement = new WaitBusActivityMovement(parent);
	WaitBusActivity* waitBusActivity = new WaitBusActivity(parent,
			parent->getMutexStrategy(), behavior, movement);
	behavior->setParentWaitBusActivity(waitBusActivity);
	movement->setParentWaitBusActivity(waitBusActivity);
	return waitBusActivity;
}

void sim_mob::medium::WaitBusActivity::increaseWaitingTime(unsigned int incrementMs) {
	waitingTime += incrementMs;
}

void sim_mob::medium::WaitBusActivity::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}

void sim_mob::medium::WaitBusActivity::increaseFailedBoardingTimes()
{
	failedBoardingTimes++;
}

void sim_mob::medium::WaitBusActivity::makeBoardingDecision(BusDriver* driver) {
	const std::vector<const sim_mob::BusStop*>* stopsVec =
			driver->getBusStopsVector();
	if (!stopsVec) {
		setBoardBus(false);
		return;
	}

	const std::string busLineID = driver->getBusLineID();
	sim_mob::SubTrip& subTrip = *(getParent()->currSubTrip);
	if(busLineID==subTrip.getBusLineID()){
		setBoardBus(true);
		return;
	}

	const sim_mob::BusStop* destStop = nullptr;
	if (getParent()->destNode.type_ == WayPoint::BUS_STOP
			&& getParent()->destNode.busStop_) {
		destStop = getParent()->destNode.busStop_;
	}

	if (!destStop) {
		setBoardBus(false);
		return;
	}

	std::vector<const sim_mob::BusStop*>::const_iterator itStop = std::find(
			stopsVec->begin(), stopsVec->end(), destStop);
	if (itStop != stopsVec->end()) {
		setBoardBus(true);
	}
}

void sim_mob::medium::WaitBusActivity::HandleParentMessage(messaging::Message::MessageType type,
		const messaging::Message& message)
{

}

std::vector<BufferedBase*> sim_mob::medium::WaitBusActivity::getSubscriptionParams() {
	return vector<BufferedBase*>();
}
}
}
