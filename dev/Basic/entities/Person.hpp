/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "Agent.hpp"
#include "roles/Role.hpp"

namespace sim_mob
{

/**
 * Basic Person class. A person may perform one of several roles which
 *  change over time. For example: Drivers, Pedestrians, and Passengers are
 *  all roles which a Person may fulfill.
 */
class Person : public sim_mob::Agent {
public:
	Person(unsigned int id=0);

	///Update Person behavior
	virtual void update(frame_t frameNumber);

	///Update a Person's subscription list.
	virtual void buildSubscriptionList();

	///Change the role of this person: Driver, Passenger, Pedestrian
	void changeRole(sim_mob::Role* newRole);
	sim_mob::Role* getRole();

private:
	//Properties
	sim_mob::Role* currRole;


};





}
