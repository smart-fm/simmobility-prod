//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * BusDriver.cpp
 *
 *  Created on: May 6, 2013
 *      Author: zhang
 *      		melani
 */

#include "BusDriver.hpp"
#include "entities/Person.hpp"

using namespace sim_mob;
using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;

sim_mob::medium::BusDriver::BusDriver(Agent* parent, MutexStrategy mtxStrat, sim_mob::medium::BusDriverBehavior* behavior, sim_mob::medium::BusDriverMovement* movement)
: sim_mob::medium::Driver(parent, mtxStrat, behavior, movement) {
	// TODO Auto-generated constructor stub

}

sim_mob::medium::BusDriver::~BusDriver() {
	// TODO Auto-generated destructor stub
}

Role* sim_mob::medium::BusDriver::clone(Person* parent) const {
	//return new sim_mob::medium::BusDriver(parent, parent->getMutexStrategy());

	BusDriverBehavior* behavior = new BusDriverBehavior(parent);
	BusDriverMovement* movement = new BusDriverMovement(parent);
	BusDriver* busdriver = new BusDriver(parent, parent->getMutexStrategy(), behavior, movement);
	behavior->setParentDriver(busdriver);
	movement->setParentDriver(busdriver);
	return busdriver;
}

std::vector<BufferedBase*> sim_mob::medium::BusDriver::getSubscriptionParams() {
	vector<BufferedBase*> res;
	//res.push_back(&(currLane_));
	//res.push_back(&(currLaneOffset_));
	//res.push_back(&(currLaneLength_));
	return res;
}

void sim_mob::medium::BusDriver::make_frame_tick_params(timeslice now)
{
	getParams().reset(now, *this);
}
