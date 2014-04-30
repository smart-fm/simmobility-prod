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
#include "entities/roles/driver/BusDriver.hpp"

namespace sim_mob {

namespace medium {

class BusStopAgent: public sim_mob::Agent {
public:
	BusStopAgent(const MutexStrategy& mtxStrat, int id);
	virtual ~BusStopAgent();

protected:

	//Virtual overrides
	virtual bool frame_init(timeslice now) {
		throw std::runtime_error(
				"frame_* methods are not required and are not implemented for Confluxes.");
	}
	virtual Entity::UpdateStatus frame_tick(timeslice now) {
		throw std::runtime_error(
				"frame_* are not required and are not implemented for Confluxes.");
	}
	virtual void frame_output(timeslice now) {
		throw std::runtime_error(
				"frame_* methods are not required and are not implemented for Confluxes.");
	}

	/**
	 * Inherited from Agent.
	 */
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId,
			event::EventPublisher* sender, const event::EventArgs& args);

public:
	/**
	 * register a new waiting people.
	 * @param person is new one who enter this bus stop
	 */
	void registerNewWaitingPerson(sim_mob::Person* person);

	/**
	 * remove a waiting people from this bus stop.
	 * @param person is the one who will be removed from this bus stop
	 */
	void removeWaitingPerson(sim_mob::Person* person);

	/**
	 * passenger alight to this bus stop.
	 * @param person is the one who will alight to this bus stop
	 */
	void alightingPassengerToStop(sim_mob::Person* person);

	/**
	 * set associate bus stop to this agent.
	 * @param bus stop is the associate to this agent
	 */
	void setAssociateBusStop(sim_mob::BusStop* stop);

	/**
	 * the getter of associate bus stop to this agent.
	 * @return bus stop is the associate to this agent
	 */
	sim_mob::BusStop* getAssociateBusStop();

	/**
	 * process the persons boarding
	 * @param Bus Driver is the associate driver which waiting people will board
	 */
	void processWaitingPersonBoarding(sim_mob::medium::BusDriver* busDriver);

	/**
	 * process the persons alighting
	 */
	void processPersonAlighting();

private:
	std::vector<sim_mob::Person*> waitingPersons;
	std::vector<sim_mob::Person*> alightingPersons;
	std::vector<sim_mob::medium::BusDriver*> parkingDrivers;
	sim_mob::BusStop* busStop;

};
}
}

