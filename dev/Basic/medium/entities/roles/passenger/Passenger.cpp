//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Passenger.hpp"
#include "PassengerFacets.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "message/MT_Message.hpp"

using std::vector;
using namespace sim_mob;

namespace sim_mob {

namespace medium {

sim_mob::medium::Passenger::Passenger(Person* parent, MutexStrategy mtxStrat,
		sim_mob::medium::PassengerBehavior* behavior,
		sim_mob::medium::PassengerMovement* movement) :
		sim_mob::Role(behavior, movement, parent, "Passenger_"),
		driver(nullptr), alightBus(false) {

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

void sim_mob::medium::Passenger::setDriver(const Driver* driver) {
	this->driver = driver;
}

const sim_mob::medium::Driver* sim_mob::medium::Passenger::getDriver() const {
	return driver;
}

void sim_mob::medium::Passenger::makeAlightingDecision(const sim_mob::BusStop* nextStop) {
	if (getParent()->destNode.type_ == WayPoint::BUS_STOP
			&& getParent()->destNode.busStop_ == nextStop) {
		setAlightBus(true);
		setDriver(nullptr);
	}
}

void sim_mob::medium::Passenger::HandleParentMessage(messaging::Message::MessageType type,
		const messaging::Message& message)
{
	switch (type) {
	case ALIGHT_BUS: {
		const BusStopMessage& msg = MSG_CAST(BusStopMessage, message);
		makeAlightingDecision(msg.nextStop);
		break;
	}
	default: {
		break;
	}
	}
}
}
}
