/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "GenConfig.h"
#include "Entity.hpp"

namespace sim_mob
{


//Forward declaration
class Signal;



/**
 * Basic Region class. Currently does nothing.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 */
class Region : public Entity {
public:
	explicit Region(unsigned int id=0);

	virtual bool update(frame_t frameNumber);

	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList) {} //Nothing for now

private:
	std::vector<sim_mob::Signal*> signals;
};

}
