//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Pedestrian.cpp
 *
 *  Created on: 2011-6-20
 *      Author: Linbo
 */

#include "Passenger.hpp"
#include "PassengerFacets.hpp"
#include "entities/Person.hpp"

using std::vector;
using namespace sim_mob;

namespace sim_mob {

namespace medium {

sim_mob::medium::Passenger::Passenger(Agent* parent, MutexStrategy mtxStrat,
		sim_mob::medium::PassengerBehavior* behavior,
		sim_mob::medium::PassengerMovement* movement) :
		sim_mob::Role(behavior, movement, parent, "Passenger_") {

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

}

}

