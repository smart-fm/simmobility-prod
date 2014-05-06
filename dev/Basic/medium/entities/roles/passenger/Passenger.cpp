//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Passenger.hpp"
#include "PassengerFacets.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/MesoEventType.hpp"

using std::vector;
using namespace sim_mob;

namespace sim_mob {

namespace medium {

sim_mob::medium::Passenger::Passenger(Agent* parent, MutexStrategy mtxStrat,
		sim_mob::medium::PassengerBehavior* behavior,
		sim_mob::medium::PassengerMovement* movement) :
		sim_mob::Role(behavior, movement, parent, "Passenger_"),
		driver(nullptr), decision(NO_DECISION) {

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

void sim_mob::medium::Passenger::setDriver(const Driver* inDriver) {
	driver = inDriver;
}

const sim_mob::medium::Driver* sim_mob::medium::Passenger::getDriver() const {
	return driver;
}

void sim_mob::medium::Passenger::setDecision(
		Decision decisionResult) {
	decision = decisionResult;
}

Decision sim_mob::medium::Passenger::getDecision() {
	return decision;
}

void sim_mob::medium::Passenger::makeAlightingDecision(const sim_mob::BusStop* nextStop) {
	if (getParent()->destNode.type_ == WayPoint::BUS_STOP
			&& getParent()->destNode.busStop_ == nextStop) {
		setDecision(ALIGHT_BUS);
		setDriver(nullptr);
	} else {
		setDecision(BOARD_BUS);
	}
}

void sim_mob::medium::Passenger::HandleParentMessage(messaging::Message::MessageType type,
		const messaging::Message& message)
{
	switch(type){
	case MSG_DECISION_PASSENGER_ALIGHTING:
		const PassengerAlightingDecisionMessageArgs& msg = MSG_CAST(PassengerAlightingDecisionMessageArgs, message);
		makeAlightingDecision(msg.nextStop);
		break;
	}
}
}
}
