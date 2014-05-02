//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Driver.hpp"
#include "BusDriverFacets.hpp"
#include "entities/misc/BusTrip.hpp"

/*
 * BusDriver.hpp
 *
 *  Created on: May 6, 2013
 *      Author: zhang huai peng
 *      		melani
 */

namespace sim_mob {

namespace medium
{


class BusDriverBehavior;
class BusDriverMovement;

class BusDriver : public sim_mob::medium::Driver {
public:
	BusDriver(Agent* parent, MutexStrategy mtxStrat,
			sim_mob::medium::BusDriverBehavior* behavior = nullptr,
			sim_mob::medium::BusDriverMovement* movement = nullptr);
	virtual ~BusDriver();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual void make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

protected:
	friend class BusDriverBehavior;
	friend class BusDriverMovement;
};

}
}


