/*
 * PassengerFacets.h
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#pragma once

#include "entities/roles/RoleFacets.hpp"
#include "entities/Person.hpp"

namespace sim_mob {
namespace medium {

class Passenger;

class PassengerBehavior: public BehaviorFacet {
public:
	explicit PassengerBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~PassengerBehavior();

	//Virtual overrides
	virtual void frame_init() {;}
	virtual void frame_tick() {;}
	virtual void frame_tick_output() {;}

	/**
	 * set parent reference to passenger role.
	 * @param parentPassenger is pointer to parent passenger role
	 */
	void setParentPassenger(sim_mob::medium::Passenger* parentPassenger);

protected:
	sim_mob::medium::Passenger* parentPassenger;
};

class PassengerMovement: public MovementFacet {
public:
	explicit PassengerMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~PassengerMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	/**
	 * set parent reference to passenger role.
	 * @param parentPassenger is pointer to parent passenger role
	 */
	void setParentPassenger(sim_mob::medium::Passenger* parentPassenger);

protected:
	sim_mob::medium::Passenger* parentPassenger;
	int totalTimeToCompleteMS;

};

}
}
