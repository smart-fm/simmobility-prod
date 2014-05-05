//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "waitBusActivity.hpp"
#include "waitBusActivityFacets.hpp"
#include "entities/Person.hpp"
#include "geospatial/BusStop.hpp"

using std::vector;
using namespace sim_mob;

namespace sim_mob {

namespace medium {

sim_mob::medium::WaitBusActivity::WaitBusActivity(Agent* parent,
		MutexStrategy mtxStrat,
		sim_mob::medium::WaitBusActivityBehavior* behavior,
		sim_mob::medium::WaitBusActivityMovement* movement) :
		sim_mob::Role(behavior, movement, parent, "WaitBusActivity_"),
		waitingTime(0), stop(nullptr), decision(NO_DECISION)
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

void sim_mob::medium::WaitBusActivity::setDecision(
		Decision decisionResult) {
	decision = decisionResult;
}

Decision sim_mob::medium::WaitBusActivity::getDecision() {
	return decision;
}

void sim_mob::medium::WaitBusActivity::increaseWaitingTime(unsigned int incrementMs) {
	waitingTime += incrementMs;
}

void sim_mob::medium::WaitBusActivity::setStop(sim_mob::BusStop* busStop) {
	stop = busStop;
}

void sim_mob::medium::WaitBusActivity::makeBoardingDecision(BusDriver* driver) {

}

}
}
