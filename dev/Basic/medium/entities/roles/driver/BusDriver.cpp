/*
 * BusDriver.cpp
 *
 *  Created on: May 6, 2013
 *      Author: zhang
 */

#include "BusDriver.hpp"

using namespace sim_mob;
using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;

sim_mob::medium::BusDriver::BusDriver(Agent* parent, MutexStrategy mtxStrat) : Driver(parent, mtxStrat) {
	// TODO Auto-generated constructor stub

}

sim_mob::medium::BusDriver::~BusDriver() {
	// TODO Auto-generated destructor stub
}


std::vector<BufferedBase*> sim_mob::medium::BusDriver::getSubscriptionParams() {
	vector<BufferedBase*> res;
	//res.push_back(&(currLane_));
	//res.push_back(&(currLaneOffset_));
	//res.push_back(&(currLaneLength_));
	return res;
}

sim_mob::UpdateParams& sim_mob::medium::BusDriver::make_frame_tick_params(timeslice now)
{
	params.reset(now, *this);
	return params;
}

void sim_mob::medium::BusDriver::frame_init(sim_mob::UpdateParams& p)
{

}
void sim_mob::medium::BusDriver::frame_tick(sim_mob::UpdateParams& p)
{

}
void sim_mob::medium::BusDriver::frame_tick_output(const sim_mob::UpdateParams& p)
{

}

