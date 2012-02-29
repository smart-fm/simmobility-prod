/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Region.hpp"

using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;


sim_mob::Region::Region(unsigned int id) : Entity(id) {
	for (unsigned int i=id*3; i<id*3+3; i++) {
		//signals.push_back(Signal(i));  //For now, don't do anything.
	}
}


UpdateStatus sim_mob::Region::update(frame_t frameNumber) {
	//Trivial. Todo: Update signals
	for (std::vector<Signal*>::iterator it=signals.begin(); it!=signals.end(); it++) {
		//trivial(it->id);  //Again, for now do nothing.
	}

	return UpdateStatus::Continue;
}


