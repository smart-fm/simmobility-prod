//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "conf/settings/DisableMPI.h"
#include "entities/roles/RoleFacets.hpp"
#include "Passenger.hpp"

namespace sim_mob
{

class Passenger;
class Bus;
class BusDriver;

class PassengerBehavior : public sim_mob::BehaviorFacet
{
public:
	explicit PassengerBehavior();
	virtual ~PassengerBehavior();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

	Passenger* getParentPassenger() const
	{
		return parentPassenger;
	}

	void setParentPassenger(Passenger* parentPassenger)
	{
		this->parentPassenger = parentPassenger;
	}

private:
	Passenger* parentPassenger;
};

class PassengerMovement : public sim_mob::MovementFacet
{
private:
	Passenger* parentPassenger;
	
	/**Stores the total time to complete the trip*/
	unsigned int totalTimeToComplete;
	
public:
	explicit PassengerMovement();
	virtual ~PassengerMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

	//mark startTimeand origin
	virtual TravelMetric & startTravelTimeMetric()
	{
	}

	//mark the destination and end time and travel time
	virtual TravelMetric & finalizeTravelTimeMetric()
	{
	}

	Passenger* getParentPassenger() const
	{
		return parentPassenger;
	}

	void setParentPassenger(Passenger* parentPassenger)
	{
		this->parentPassenger = parentPassenger;
	}
};

}
