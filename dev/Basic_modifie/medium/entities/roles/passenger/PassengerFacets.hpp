/*
 * PassengerFacets.h
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#pragma once

#include "entities/conflux/Conflux.hpp"
#include "entities/roles/RoleFacets.hpp"

namespace sim_mob
{
namespace medium
{

class Passenger;

class PassengerBehavior : public BehaviorFacet
{
public:
	explicit PassengerBehavior();
	virtual ~PassengerBehavior();

	//Virtual overrides

	virtual void frame_init()
	{
	}

	virtual void frame_tick()
	{
	}

	virtual std::string frame_tick_output()
	{
		return std::string();
	}

	/**
	 * set parent reference to passenger role.
	 * @param parentPassenger is pointer to parent passenger role
	 */
	void setParentPassenger(Passenger* parentPassenger);

protected:
	Passenger* parentPassenger;
};

class PassengerMovement : public MovementFacet
{
public:
	explicit PassengerMovement();
	virtual ~PassengerMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();
	virtual Conflux* getDestinationConflux() const;

	/**
	 * set parent reference to passenger role.
	 * @param parentPassenger is pointer to parent passenger role
	 */
	void setParentPassenger(Passenger* parentPassenger);
	TravelMetric & startTravelTimeMetric();
	TravelMetric & finalizeTravelTimeMetric();

protected:
	Passenger* parentPassenger;
	unsigned int totalTimeToCompleteMS;
};

}
}
