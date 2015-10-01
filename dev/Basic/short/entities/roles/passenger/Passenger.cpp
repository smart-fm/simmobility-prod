//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Passenger.cpp
 * Created on: 2012-12-20
 * Author: Meenu
 */
#include "Passenger.hpp"
//#include "PassengerFacets.hpp"
#include "entities/Person.hpp"
#include "entities/vehicle/Bus.hpp"
#include "entities/Person_ST.hpp"

using namespace sim_mob;
using std::vector;
using std::cout;
using std::map;
using std::string;

sim_mob::Passenger::Passenger(Person_ST *parent, MutexStrategy mtxStrat, sim_mob::PassengerBehavior* behavior, sim_mob::PassengerMovement* movement,
							  Role::Type roleType_, std::string roleName) :
Role(parent, behavior, movement, parent, roleName, roleType_), BoardedBus(mtxStrat, false), AlightedBus(mtxStrat, false), busdriver(mtxStrat, nullptr),
params(parent->getGenerator()), waitingTimeAtStop(0)
{
}

void sim_mob::Passenger::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}

Role* sim_mob::Passenger::clone(sim_mob::Person_ST *parent) const
{
	PassengerBehavior* behavior = new PassengerBehavior();
	PassengerMovement* movement = new PassengerMovement();
	Passenger* passenger = new Passenger(parent, parent->getMutexStrategy(), behavior, movement);
	behavior->setParentPassenger(passenger);
	movement->setParentPassenger(passenger);
	return passenger;
}

std::vector<sim_mob::BufferedBase*> sim_mob::Passenger::getSubscriptionParams()
{
	std::vector<sim_mob::BufferedBase*> res;
	res.push_back(&(BoardedBus));
	res.push_back(&(AlightedBus));
	res.push_back(&(busdriver));
	return res;
}
