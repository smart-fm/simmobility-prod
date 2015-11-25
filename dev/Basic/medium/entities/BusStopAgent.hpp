//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <boost/unordered_map.hpp>
#include <list>
#include "entities/Agent.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/roles/waitBusActivity/WaitBusActivity.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "geospatial/network/PT_Stop.hpp"

namespace sim_mob
{
namespace medium
{
/**
 * Agent to manage activities at a bus stop
 *
 * \author Zhang Huai Peng
 * \author Harish Loganathan
 */
class BusStopAgent: public sim_mob::Agent
{
public:
	typedef boost::unordered_map<const BusStop*, BusStopAgent*> BusStopAgentsMap; // there can be a lot of bus stops in the road network. unordered_map is faster.

	BusStopAgent(const MutexStrategy& mtxStrat, int id, const sim_mob::BusStop* stop, SegmentStats* stats);
	virtual ~BusStopAgent();

	const sim_mob::BusStop* getBusStop() const;
	const SegmentStats* getParentSegmentStats() const;

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
	unsigned int getBoardingNum(sim_mob::medium::BusDriver* busDriver) const;

	/**
	 * checks whether the bus stop can accommodate a vehicle of a given length
	 * @param length length of the vehicle
	 * @returns true if vehicle can be accommodated; false otherwise
	 */
	bool canAccommodate(double vehicleLength) const;

	/**
	 * accepts the incoming bus driver and co-ordinates the boarding activity
	 * @param busDriver the driver of the arriving bus
	 * @return true if bus driver is accepted. false otherwise.
	 */
	bool handleBusArrival(BusDriver* busDriver);

	/**
	 * removes bus driver from the local servingDrivers list
	 * @param busDriver the driver of the departing bus
	 * @return true if busDriver was removed correctly; false otherwise.
	 */
	bool handleBusDeparture(BusDriver* busDriver);

	/**
	 * store waiting time
	 * @param waitingActivity is pointer to the waiting people
	 */
	void storeWaitingTime(sim_mob::medium::WaitBusActivity* waitingActivity) const;

	/**
	 * returns number of people waiting for buses in this stop
	 */
	unsigned int getWaitingCount() const;

	/**
	 * finds the BusStopAgent corresponding to a bus stop.
	 * @param busstop stop under consideration
	 * @returns pointer to bus stop agent corresponding to busstop
	 */
	static BusStopAgent* getBusStopAgentForStop(const BusStop* busstop);

	/**
	 * adds bus stop agent to the static allBusstopAgents
	 */
	static void registerBusStopAgent(BusStopAgent* busstopAgent);

	/**
	 * remove all bus stops agent.
	 */
	static void removeAllBusStopAgents();

protected:
	//Virtual overrides
	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);
	virtual bool isNonspatial();
	virtual void load(const std::map<std::string, std::string>& configProps);
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

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
	/** global static bus stop agents lookup table*/
	static BusStopAgentsMap allBusstopAgents;
	/** list of persons waiting at this stop*/
	std::list<sim_mob::medium::WaitBusActivity*> waitingPersons;
	/** list of persons who just alighted (in current tick) at this stop*/
	std::list<sim_mob::medium::Passenger*> alightingPersons;
	/** list of bus drivers currently serving the stop*/
	std::list<sim_mob::medium::BusDriver*> servingDrivers;
	/**bus stop managed by this agent*/
	const sim_mob::BusStop* busStop;
	/**segment stats containing this bus stop*/
	SegmentStats* parentSegmentStats;
	/**record last boarding number for a given bus*/
	std::map<sim_mob::medium::BusDriver*, unsigned int> lastBoardingRecorder;
	/**available length in m for incoming vehicles*/
	double availableLength;
	/**current time in milliseconds from start of simulation*/
	unsigned int currentTimeMS;
};
}
}
