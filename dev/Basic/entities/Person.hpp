/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "Agent.hpp"
#include "roles/Role.hpp"
#include "roles/driver/Driver.hpp"

namespace sim_mob
{

class TripChain;
class AgentPackageManager;

/**
 * Basic Person class. A person may perform one of several roles which
 *  change over time. For example: Drivers, Pedestrians, and Passengers are
 *  all roles which a Person may fulfill.
 */
class Person : public sim_mob::Agent {
public:
	Person(int id=-1);

	///Update Person behavior
	virtual void update(frame_t frameNumber);

	virtual void output(frame_t frameNumber);

	///Update a Person's subscription list.
	virtual void buildSubscriptionList();

	///Change the role of this person: Driver, Passenger, Pedestrian
	void changeRole(sim_mob::Role* newRole);
	sim_mob::Role* getRole() const;

	///Set this person's trip chain
	void setTripChain(sim_mob::TripChain* newTripChain) { currTripChain = newTripChain; }
	sim_mob::TripChain* getTripChain() { return currTripChain; }

private:
	//Properties
	sim_mob::Role* currRole;
	sim_mob::TripChain* currTripChain;

	//add by xuyan
public:
	friend class AgentPackageManager;
};





}
