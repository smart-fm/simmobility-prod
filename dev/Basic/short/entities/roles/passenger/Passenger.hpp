//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/Person_ST.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/waitBusActivity/WaitBusActivity.hpp"
#include "geospatial/network/WayPoint.hpp"

namespace sim_mob
{

class Agent;
class Person;
class BusStop;
class PassengerBehavior;
class PassengerMovement;
class Driver;

class Passenger : public Role<Person_ST>
{
private:
	/**Indicates whether the passenger has decided to alight*/
	bool alightVehicle;

	/** starting point of passenger - for travel time storage */
	WayPoint startPoint;

	/** ending node of passenger - for travel time storage */
	WayPoint endPoint;
	
public:
	explicit Passenger(Person_ST *parent, PassengerBehavior *behavior = nullptr, PassengerMovement *movement = nullptr,
					std::string roleName = std::string("Passenger_"), Role<Person_ST>::Type roleType = Role<Person_ST>::RL_PASSENGER);

	virtual ~Passenger()
	{
	}

	//Virtual overrides
	virtual Role<Person_ST>* clone(Person_ST *parent) const;
	
	virtual std::vector<BufferedBase*> getSubscriptionParams();
	
	/**
	 * Make alighting decision
	 * 
	 * @param nextStop is the stop at which the bus will arrive at next
	 * @param driver is the driver of the bus
	 */
	void makeAlightingDecision(const BusStop *nextStop, BusDriver *driver);

	/**
	 * Collect travel time for current role
	 */
	virtual void collectTravelTime();
	
	/**
	 * Message handler which provides a chance to handle message transfered from parent agent.
	 * 
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message);
	
	virtual void make_frame_tick_params(timeslice now)
	{
	}

	bool canAlightVehicle() const
	{
		return alightVehicle;
	}

	void setAlightVehicle(bool alight)
	{
		this->alightVehicle = alight;
	}

	const WayPoint& getEndPoint() const
	{
		return endPoint;
	}

	void setEndPoint(const WayPoint& endPoint)
	{
		this->endPoint = endPoint;
	}

	const WayPoint& getStartPoint() const
	{
		return startPoint;
	}

	void setStartPoint(const WayPoint& startPoint)
	{
		this->startPoint = startPoint;
	}
	
	friend class PassengerBehavior;
	friend class PassengerMovement;
};

}
