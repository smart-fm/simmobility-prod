#pragma once

#include <vector>

#include "Agent.hpp"
#include "../roles/Role.hpp"

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

	///Update person behavior
	virtual void update(frame_t frameNumber);

	///Update this person's subscribed data members.
	/// \todo
	/// It might make more sense to generalize this somehow.
	virtual void subscribe(sim_mob::BufferedDataManager* mgr, bool isNew);

	void changeRole(sim_mob::Role* newRole);

private:
	//Properties
	sim_mob::Role* currRole;


};





}
