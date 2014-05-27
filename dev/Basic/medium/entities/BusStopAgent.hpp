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
#include "entities/conflux/SegmentStats.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/roles/waitBusActivity/waitBusActivity.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "geospatial/BusStop.hpp"

namespace sim_mob {
namespace medium {
/**
 * Agent to manage activities at a bus stop
 *
 * \author Zhang Huai Peng
 * \author Harish Loganathan
 */
class BusStopAgent: public sim_mob::Agent {
public:
	typedef boost::unordered_map<const BusStop*, BusStopAgent*> BusStopAgentsMap; // there can be a lot of bus stops in the road network. unordered_map is faster.

	BusStopAgent(const MutexStrategy& mtxStrat, int id,
			const sim_mob::BusStop* stop, SegmentStats* stats);
	virtual ~BusStopAgent();

	const sim_mob::BusStop* getBusStop() const;

	/**
	 * register a new waiting person.
	 * @param person person who wants to enter this bus stop
	 */
	void registerWaitingPerson(sim_mob::medium::WaitBusActivity* waitingActivity);

	/**
	 * remove a waiting people from this bus stop.
	 * @param person person to be removed from this bus stop
	 */
	void removeWaitingPerson(sim_mob::medium::WaitBusActivity* waitingActivity);

	/**
	 * add person who is alighting at this stop
	 * @param person person who is alighting at this bus stop
	 */
	void addAlightingPerson(sim_mob::medium::Passenger* passenger);

	/**
	 * get the number of boarding people
	 * @param Bus Driver is the associate driver which waiting people will board
	 * @returns number of boarding people
	 */
	int getBoardingNum(sim_mob::medium::BusDriver* busDriver) const;

	/**
	 * finds the BusStopAgent corresponding to a bus stop.
	 * @param busstop stop under consideration
	 * @returns pointer to bus stop agent corresponding to busstop
	 */
	static BusStopAgent* findBusStopAgentByBusStop(const BusStop* busstop);

	/**
	 * adds bus stop agent to the static allBusstopAgents
	 */
	static void registerBusStopAgent(BusStopAgent* busstopAgent);

	/**
	 * checks whether the bus stop can accommodate a vehicle of a given length
	 * @param length length of the vehicle
	 * @returns true if vehicle can be accommodated; false otherwise
	 */
	bool canAccommodate(const double vehicleLength);

protected:
	//Virtual overrides
	virtual bool frame_init(timeslice now) { throw std::runtime_error("frame_init() is not required and is not implemented for BusStopAgent."); }
	virtual Entity::UpdateStatus frame_tick(timeslice now) { throw std::runtime_error("frame_tick() is not required and are not implemented for BusStopAgent."); }
	virtual void frame_output(timeslice now) { throw std::runtime_error("frame_output() is not required and is not implemented for BusStopAgent."); }
	virtual bool isNonspatial(){
		return false;
	}
	virtual void load(const std::map<std::string, std::string>& configProps) {
	}

	//Inherited from Agent.
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId,
			event::EventPublisher* sender, const event::EventArgs& args);

	//Inherited from MessageHandler.
	virtual void HandleMessage(messaging::Message::MessageType type,
			const messaging::Message& message);

	/**
	 * process the persons boarding
	 * @param Bus Driver is the associate driver which waiting people will board
	 */
	void boardWaitingPersons(sim_mob::medium::BusDriver* busDriver);

	/**
	 * accepts an arriving driver.
	 * Adds bus driver to the local servingDrivers list
	 * Adds the person with the BusDriver role to the segment stats busDrivers list
	 * @param driver the driver of the bus to be accepted in the stop
	 * @returns true if bus driver is accepted; false otherwise
	 */
	bool acceptBusDriver(BusDriver* driver);

	/**
	 * removes departing driver.
	 * removes bus driver from the local servingDrivers list
	 * removes the person with BusDriver role from the segment stats busDrivers list
	 * @param driver the driver of the bus to be removed from the stop
	 * @returns true if bus driver is removed; false otherwise
	 */
	bool removeBusDriver(BusDriver* driver);


private:
	static BusStopAgentsMap allBusstopAgents;
	std::list<sim_mob::medium::WaitBusActivity*> waitingPersons;
	std::list<sim_mob::medium::Passenger*> alightingPersons;
	std::list<sim_mob::medium::BusDriver*> servingDrivers;
	/**bus stop managed by this agent*/
	const sim_mob::BusStop* busStop;
	/**segment stats containing this bus stop*/
	SegmentStats* parentSegmentStats;
	/**record last boarding number for a given bus*/
	std::map<sim_mob::medium::BusDriver*, int> lastBoardingRecorder;
	/**available length in cm for incoming vehicles*/
	double availableLength;

};
}
}

