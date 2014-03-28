//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "entities/roles/Role.hpp"
#include "PassengerFacets.hpp"

namespace sim_mob
{

class Agent;
class Person;

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
class Passenger : public sim_mob::Role {
public:

	explicit Passenger(Agent* parent, MutexStrategy mtxStrat, sim_mob::medium::PassengerBehavior* behavior = nullptr, sim_mob::medium::PassengerMovement* movement = nullptr);

	virtual ~Passenger() {}

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	virtual void make_frame_tick_params(timeslice now) { throw std::runtime_error("make_frame_tick_params not implemented in Passenger."); }
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams() { throw std::runtime_error("getSubscriptionParams not implemented in Passenger."); }

	void setAssociateDriver(Driver* driver);

	Driver* getAssociateDriver();

private:
	friend class PassengerBehavior;
	friend class PassengerMovement;

	Driver* associateDriver;
};


}}
