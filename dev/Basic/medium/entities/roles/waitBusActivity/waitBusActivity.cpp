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

sim_mob::medium::WaitBusActivity::WaitBusActivity(Agent* parent,
		MutexStrategy mtxStrat,
		sim_mob::medium::WaitBusActivityBehavior* behavior,
		sim_mob::medium::WaitBusActivityMovement* movement) :
		sim_mob::Role(behavior, movement, parent, "WaitBusActivity_"), waitingTimeAtBusStop(
				0), stopSite(nullptr), decisionResult(MAKE_DECISION_NORESULT) {

}

Role* sim_mob::medium::WaitBusActivity::clone(Person* parent) const {
	WaitBusActivityBehavior* behavior = new WaitBusActivityBehavior(parent);
	WaitBusActivityMovement* movement = new WaitBusActivityMovement(parent);
	WaitBusActivity* waitBusActivity = new WaitBusActivity(parent,
			parent->getMutexStrategy(), behavior, movement);
	behavior->setParentWaitBusActivity(waitBusActivity);
	movement->setParentWaitBusActivity(waitBusActivity);
	return waitBusActivity;
}

void sim_mob::medium::WaitBusActivity::setDecisionResult(Decision decision) {
	decisionResult = decision;
}

Decision sim_mob::medium::WaitBusActivity::getDecisionResult() {
	return decisionResult;
}

void sim_mob::medium::WaitBusActivity::increaseWaitingTime(unsigned int ms) {
	waitingTimeAtBusStop += ms;
}

void sim_mob::medium::WaitBusActivity::setStopSite(sim_mob::BusStop* stop) {
	stopSite = stop;
}

void sim_mob::medium::WaitBusActivity::makeBoardingDecision(BusDriver* driver) {
	const std::vector<const sim_mob::BusStop*>* stopsVec =
			driver->getBusStopsVector();
	if (!stopsVec) {
		setDecisionResult(MAKE_DECISION_NORESULT);
		return;
	}

	const sim_mob::BusStop* destStop = nullptr;
	if (getParent()->destNode.type_ == WayPoint::BUS_STOP
			&& getParent()->destNode.busStop_) {
		destStop = getParent()->destNode.busStop_;
	}

	if (!destStop) {
		setDecisionResult(MAKE_DECISION_NORESULT);
		return;
	}

	std::vector<const sim_mob::BusStop*>::const_iterator itStop;
	itStop = std::find(stopsVec->begin(), stopsVec->end(), destStop);
	if (itStop != stopsVec->end()) {
		setDecisionResult(MAKE_DECISION_BOARDING);
	}
}
}
}
