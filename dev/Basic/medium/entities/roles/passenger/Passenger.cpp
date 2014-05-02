//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Passenger.hpp"
#include "PassengerFacets.hpp"
#include "entities/Person.hpp"
#include "../driver/Driver.hpp"

using std::vector;
using namespace sim_mob;

namespace sim_mob {

namespace medium {

sim_mob::medium::Passenger::Passenger(Agent* parent, MutexStrategy mtxStrat,
		sim_mob::medium::PassengerBehavior* behavior,
		sim_mob::medium::PassengerMovement* movement) :
		sim_mob::Role(behavior, movement, parent, "Passenger_"),
		associateDriver(nullptr), decisionResult(MAKE_DECISION_NORESULT) {

}

Role* sim_mob::medium::Passenger::clone(Person* parent) const {

	PassengerBehavior* behavior = new PassengerBehavior(parent);
	PassengerMovement* movement = new PassengerMovement(parent);
	Passenger* passenger = new Passenger(parent, parent->getMutexStrategy(),
			behavior, movement);
	behavior->setParentPassenger(passenger);
	movement->setParentPassenger(passenger);
	return passenger;
}

void sim_mob::medium::Passenger::setAssociateDriver(const Driver* driver) {
	associateDriver = driver;
}

const sim_mob::medium::Driver* sim_mob::medium::Passenger::getAssociateDriver() const {
	return associateDriver;
}

void sim_mob::medium::Passenger::setDecisionResult(
		Decision decision) {
	decisionResult = decision;
}

Decision sim_mob::medium::Passenger::getDecisionResult() {
	return decisionResult;
}

void sim_mob::medium::Passenger::makeAlightingDecision(BusStop* nextStop) {
	if (getParent()->destNode.type_ == WayPoint::BUS_STOP
			&& getParent()->destNode.busStop_ == nextStop) {
		setDecisionResult(MAKE_DECISION_ALIGHTING);
		setAssociateDriver(nullptr);
	} else {
		setDecisionResult(MAKE_DECISION_NORESULT);
	}
}
}
}
