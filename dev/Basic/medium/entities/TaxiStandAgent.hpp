/*
 * TaxiStandAgent.hpp
 *
 *  Created on: Nov 7, 2016
 *      Author: zhang huai peng
 */

#ifndef TAXISTANDAGENT_HPP_
#define TAXISTANDAGENT_HPP_
#include "entities/Agent.hpp"
#include "Person_MT.hpp"
#include <map>
#include <queue>
namespace sim_mob
{
class TaxiStand;
namespace medium
{
class TaxiDriver;
class TaxiStandAgent : public sim_mob::Agent {
public:
	TaxiStandAgent(const MutexStrategy& mtxStrat, int id, const TaxiStand* stand, SegmentStats* stats);
	virtual ~TaxiStandAgent();
	/**
	 * set a taxi-stand pointer to taxiStand
	 * @param stand is a pointer to a taxi-stand object
	 */
	void setTaxiStand(const TaxiStand* stand);
	/**
	 * get a associated pointer to taxi-stand
	 * @return a pointer to the associated taxi-stand
	 */
	const TaxiStand* getTaxiStand() const;
	/**
	 * add a waiting person to current taxi-stand
	 * @param person a pointer to one waiting person.
	 */
	void addWaitingPerson(medium::Person_MT* person);
	/**
	 * register a taxi-stand agent for lookup purpose.
	 * @param agent is a pointer to a taxi-stand agent
	 */
	static void registerTaxiStandAgent(TaxiStandAgent* agent);
	/**
	 * get taxi-stand agent from a given taxi-stand
	 * @param stand is a pointer to a taxi-stand
	 */
	static  TaxiStandAgent* getTaxiStandAgent(const TaxiStand* stand);
	/**
	 * get taxi-stand from a given taxi-stand id
	 * @param stand is a pointer to a taxi-stand
	 */
	static const TaxiStand* getTaxiStand(int standId);
	/**
	 * accept a queuing taxi-driver inside current stand
	 * @param driver is pointer to the person who drive this taxi
	 * @return true if current stand has space for taxi, otherwise false
	 */
	bool acceptTaxiDriver(Person_MT* driver);
	/**
	 * pick up a waiting person at current stand
	 * @return a available person.
	 */
	Person_MT* pickupOneWaitingPerson();
	/**
	 * store waiting time at current stand
	 * @param person is a pointer to a person with waiting activity role
	 */
	void storeWaitingTime(Person_MT* waitingPerson) const;

	bool isTaxiFirstInQueue(TaxiDriver *taxiDriver);

	void setParentConflux();
	Conflux * getParentConflux();

private:
	/**record associated taxi-stand*/
	const TaxiStand* taxiStand;
	/**record the waiting people at this stand*/
	std::deque<Person_MT*> waitingPeople;
	/**record the queuing taxi-drivers at current stand*/
	std::deque<Person_MT*> queuingDrivers;
	/** global static taxi stand agents lookup table*/
	static std::map<const TaxiStand*, TaxiStandAgent*> allTaxiStandAgents;
	/**segment stats containing this taxi stand*/
	SegmentStats* parentSegmentStats;
	/**current time in milliseconds*/
	unsigned int currentTimeMS;
	/**default capacity for queuing drivers*/
	const unsigned int capacity;

	Conflux *parentConflux;
protected:
	//Virtual overrides
	virtual Entity::UpdateStatus frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);
	virtual bool isNonspatial();
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);
};
}
}
#endif /* TAXISTANDAGENT_HPP_ */
