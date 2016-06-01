//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Passenger.cpp
 * Created on: 2012-12-20
 * Author: Meenu
 */
#include "Passenger.hpp"
#include "PassengerFacets.hpp"
#include "entities/Person_ST.hpp"
#include "entities/vehicle/Bus.hpp"

using namespace sim_mob;
using std::vector;
using std::cout;
using std::map;
using std::string;

Passenger::Passenger(Person_ST *parent, MutexStrategy mtxStrat, PassengerBehavior* behavior, PassengerMovement* movement, Role<Person_ST>::Type roleType_, std::string roleName) :
Role(parent, behavior, movement, roleName, roleType_), BoardedBus(mtxStrat, false), AlightedBus(mtxStrat, false), busdriver(mtxStrat, nullptr),
waitingTimeAtStop(0)
{
}

void Passenger::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}

Role<Person_ST>* Passenger::clone(Person_ST *parent) const
{
	PassengerBehavior* behavior = new PassengerBehavior();
	PassengerMovement* movement = new PassengerMovement();
	Passenger* passenger = new Passenger(parent, parent->getMutexStrategy(), behavior, movement);
	
	behavior->setParentPassenger(passenger);
	movement->setParentPassenger(passenger);
	
	return passenger;
}

std::vector<BufferedBase*> Passenger::getSubscriptionParams()
{
	std::vector<BufferedBase*> res;
	res.push_back(&(BoardedBus));
	res.push_back(&(AlightedBus));
	res.push_back(&(busdriver));
	return res;
}
