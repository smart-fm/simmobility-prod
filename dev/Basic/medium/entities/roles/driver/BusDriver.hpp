/* Copyright Singapore-MIT Alliance for Research and Technology */
#pragma once

#include "Driver.hpp"

/*
 * BusDriver.hpp
 *
 *  Created on: May 6, 2013
 *      Author: zhang huai peng
 */

namespace sim_mob {

namespace medium
{

#ifndef BUSDRIVER_HPP_
#define BUSDRIVER_HPP_

class BusDriver : public sim_mob::medium::Driver {
public:
	BusDriver(Agent* parent, MutexStrategy mtxStrat);
	virtual ~BusDriver();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now) { throw std::runtime_error("frame_tick_output_mpi not implemented in Driver."); }
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

protected:
	virtual Vehicle* initializePath(bool allocateVehicle);


};

#endif /* BUSDRIVER_HPP_ */

}
}


