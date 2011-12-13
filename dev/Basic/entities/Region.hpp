/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "GenConfig.h"
#include "Entity.hpp"

namespace sim_mob
{

/**
 * Traffic signal.
 */
/*
struct Signal {
	unsigned int id;
	Signal(unsigned int id=0) : id(id) {}
};*/

//Forward declaration
class Signal;



/**
 * Basic Region class. Currently does nothing.
 */
class Region : public Entity {
public:
	Region(unsigned int id=0);

	virtual bool update(frame_t frameNumber);

	virtual void buildSubscriptionList() {} //Nothing for now

private:
	std::vector<sim_mob::Signal*> signals;
};

}
