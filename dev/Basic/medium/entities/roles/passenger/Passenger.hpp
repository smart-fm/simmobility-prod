//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/roles/Role.hpp"
#include "entities/roles/waitBusActivity/waitBusActivity.hpp"
#include "geospatial/Node.hpp"
#include "PassengerFacets.hpp"

namespace sim_mob
{

class Agent;
class Person;
class BusStop;

namespace medium
{

class PassengerBehavior;
class PassengerMovement;
class Driver;

/**
 * A medium-term Passenger.
 * \author Seth N. Hetu
 * \author zhang huai peng
 */
class Passenger : public sim_mob::Role<Person_MT>
{
public:

	explicit Passenger(Person_MT *parent, 
					sim_mob::medium::PassengerBehavior* behavior = nullptr,
					sim_mob::medium::PassengerMovement* movement = nullptr,
					std::string roleName = std::string("Passenger_"),
					Role::Type roleType = Role::RL_PASSENGER);

	virtual ~Passenger()
	{
	}

	//Virtual overrides
	virtual sim_mob::Role* clone(sim_mob::Person_MT *parent) const;

	virtual void make_frame_tick_params(timeslice now)
	{
	}
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	/**
	 * make a decision for alighting.
	 * @param nextStop is the next stop which bus will arrive at
	 */
	void makeAlightingDecision(const sim_mob::BusStop* nextStop);

	/**
	 * message handler which provide a chance to handle message transfered from parent agent.
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message);

	/**
	 * collect travel time for current role
	 */
	virtual void collectTravelTime();

	bool canAlightBus() const
	{
		return alightBus;
	}

	void setAlightBus(bool alightBus)
	{
		this->alightBus = alightBus;
	}

	const sim_mob::Node* getStartNode() const
	{
		return startNode;
	}

	void setStartNode(const sim_mob::Node* startNode)
	{
		this->startNode = startNode;
	}

	const sim_mob::Node* getEndNode() const
	{
		return endNode;
	}

	void setEndNode(const sim_mob::Node* endNode)
	{
		this->endNode = endNode;
	}

	const sim_mob::medium::Driver* getDriver() const
	{
		return driver;
	}

	void setDriver(const Driver* driver)
	{
		this->driver = driver;
	}

private:
	friend class PassengerBehavior;
	friend class PassengerMovement;

	/** Driver who is driving the vehicle of this passenger*/
	const Driver* driver;

	/**flag to indicate whether the passenger has decided to alight the bus*/
	bool alightBus;

	/** starting node of passenger - for travel time storage */
	const sim_mob::Node* startNode;

	const sim_mob::Node* endNode;
};

}
}
