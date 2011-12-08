/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "entities/roles/Role.hpp"
#include "buffering/BufferedDataManager.hpp"

namespace sim_mob
{

/**
 * A Person in the Passenger role is likely just waiting for his or her bus stop.
 *
 *  \note
 *  This is a skeleton class. All functions are defined in this header file.
 *  When this class's full functionality is added, these header-defined functions should
 *  be moved into a separate cpp file.
 */
class Passenger : public sim_mob::Role {
public:
	virtual void update(frame_t frameNumber) {

	}

	virtual void output(frame_t frameNumber) const
	{}

	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams()
	{
		std::vector<sim_mob::BufferedBase*> res;
		return res;
	}

};



}
