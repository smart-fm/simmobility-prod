#pragma once

#include <vector>

#include "../constants.h"
#include "Entity.hpp"

namespace sim_mob
{

/**
 * Traffic signal.
 */
struct Signal {
	unsigned int id;
	Signal(unsigned int id=0) : id(id) {}
};



/**
 * Basic Region class. Currently does nothing.
 */
class Region : public Entity {
public:
	Region(unsigned int id=0);

	virtual void update(frame_t frameNumber);
	virtual void subscribe(BufferedDataManager* mgr, bool isNew) {} //Nothing for now.

private:
	std::vector<Signal> signals;
};

}
