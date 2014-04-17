//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


/*
 * BusStopAgent.hpp
 *
 *  Created on: 17 Apr, 2014
 *      Author: zhang huai peng
 */

#pragma once

#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "../roles/driver/BusDriver.hpp"

namespace sim_mob
{

namespace medium
{

class BusStopAgent : public sim_mob::Agent {
public:
	BusStopAgent(const MutexStrategy& mtxStrat, int id);
	virtual ~BusStopAgent();


protected:

	virtual bool frame_init(timeslice now) { throw std::runtime_error("frame_* methods are not required and are not implemented for Confluxes."); }
	virtual Entity::UpdateStatus frame_tick(timeslice now) { throw std::runtime_error("frame_* are not required and are not implemented for Confluxes."); }
	virtual void frame_output(timeslice now) { throw std::runtime_error("frame_* methods are not required and are not implemented for Confluxes."); }

	/**
	 * Inherited from Agent.
	 */
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);

private:
	std::vector<sim_mob::Person*> waitingPersons;
	std::vector<sim_mob::medium::BusDriver*> parkingDrivers;
	sim_mob::BusStop* busStop;

};

}

}

