//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/unordered_map.hpp>
#include <list>

#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/roles/waitBusActivity/WaitBusActivity.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "geospatial/network/PT_Stop.hpp"

namespace sim_mob
{

class BusStopAgent: public Agent
{
private:
	//There can be a lot of bus stops in the road network. unordered_map is faster.
	typedef boost::unordered_map<const BusStop*, BusStopAgent*> BusStopAgentsMap;
	
	/**Bus stop agents lookup table*/
	static BusStopAgentsMap allBusstopAgents;
	
	/**List of persons waiting at this stop*/
	std::list<WaitBusActivity *> waitingPersons;
	
	/**List of persons who alighted in the current tick at this stop*/
	std::list<Passenger *> alightingPersons;
	
	/**Bus stop managed by this agent*/
	const BusStop *busStop;
	
	/**Record last boarding number for a given bus*/
	std::map<BusDriver *, unsigned int> lastBoardingRecorder;
	
	/**Current time in milliseconds from start of simulation*/
	unsigned int currentTimeMS;
	
protected:
	
	//Virtual overrides
	virtual Entity::UpdateStatus frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);
	virtual bool isNonspatial();
	virtual void load(const std::map<std::string, std::string> &configProps);
	virtual void onEvent(event::EventId eventId, event::Context ctxId, event::EventPublisher *sender, const event::EventArgs &args);
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message &message);
	
public:
	BusStopAgent(const MutexStrategy &mtxStrat, int id, const BusStop *stop);
	virtual ~BusStopAgent();

	const BusStop* getBusStop() const;

	/**
	 * Registers a new waiting person.
	 * 
	 * @param person person who wants to enter this bus stop
	 */
	void registerWaitingPerson(WaitBusActivity *waitingActivity);

	/**
	 * Adds a person who is alighting at this stop
	 * 
	 * @param person person who is alighting at this bus stop
	 */
	void addAlightingPerson(Passenger *passenger);

	/**
	 * Gets the number of boarding people
	 * 
	 * @param busDriver the driver associated with the bus, which the waiting people will board
	 * 
	 * @returns number of boarding people
	 */
	unsigned int getBoardingNum(BusDriver *busDriver) const;

	/**
	 * For all persons waiting at the bus stop, this method checks which persons need to board the bus
	 * and begin the boarding process
	 * 
	 * @param busDriver the driver associated with the bus, which the waiting people will board
	 * 
	 * @return total time taken for boarding
	 */
	double boardWaitingPersons(BusDriver *busDriver);

	/**
	 * Stores the waiting time
	 * 
	 * @param waitingActivity is pointer to the waiting people
	 */
	void storeWaitingTime(WaitBusActivity *waitingActivity, const std::string &busLine) const;

	/**
	 * Returns number of people waiting for buses in this stop
	 */
	unsigned int getWaitingCount() const;

	/**
	 * Finds the BusStopAgent corresponding to a bus stop.
	 * 
	 * @param busstop stop under consideration
	 * 
	 * @returns pointer to bus stop agent corresponding to the bus stop
	 */
	static BusStopAgent* getBusStopAgentForStop(const BusStop *busStop);

	/**
	 * Adds bus stop agent to the static allBusstopAgents
	 * 
	 * @param busStopAgent agent to be registered
	 * 
	 * @param workGroup the work group the agent is to be added to
	 */
	static void registerBusStopAgent(BusStopAgent *busStopAgent, WorkGroup &workGroup);

	/**
	 * Removes all bus stops agent.
	 */
	static void removeAllBusStopAgents();
};

}
