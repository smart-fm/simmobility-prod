#pragma once

#include <vector>

#include "Agent.hpp"

namespace sim_mob
{

/**
 * Basic Person class. A person may perform one of several roles which
 *  change over time. For example: Drivers, Pedestrians, and Passengers are
 *  all roles which a Person may fulfill.
 *
 *  \note
 *  This is a skeleton class. All functions are defined in this header file.
 *  When this class's full functionality is added, these header-defined functions should
 *  be moved into a separate cpp file.
 */
class Person : public sim_mob::Agent {
public:
	Person(unsigned int id=0) : currRole(NULL) {
	}

	///Update person behavior
	virtual void update(frame_t frameNumber) {
		if (currRole!=NULL) {
			currRole->update();
		}
	}

	///Update this person's subscribed data members.
	/// \todo
	/// It might make more sense to generalize this somehow.
	virtual void subscribe(sim_mob::BufferedDataManager* mgr, bool isNew) {

	}

	void changeRole(sim_mob::Role* newRole) {
		this->currRole = newRole;
	}

private:
	//Properties
	sim_mob::Role* currRole;


};





}
