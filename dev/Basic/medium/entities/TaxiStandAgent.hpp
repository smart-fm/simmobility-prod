/*
 * TaxiStandAgent.hpp
 *
 *  Created on: Nov 7, 2016
 *      Author: fm-simmobility
 */

#ifndef TAXISTANDAGENT_HPP_
#define TAXISTANDAGENT_HPP_
#include "entities/Agent.hpp"
#include "geospatial/network/TaxiStand.hpp"
#include "Person_MT.hpp"
#include <map>
namespace sim_mob
{
class TaxiStandAgent : public sim_mob::Agent {
public:
	TaxiStandAgent(const MutexStrategy& mtxStrat, int id, const TaxiStand* stand);
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
	static TaxiStandAgent* getTaxiStandAgent(const TaxiStand* stand);

private:
	/**record associated taxi-stand*/
	const TaxiStand* taxiStand;
	/**record the waiting people at this stand*/
	std::queue<medium::Person_MT*> waitingPeople;
	/** global static taxi stand agents lookup table*/
	static std::map<const TaxiStand*, TaxiStandAgent*> allTaxiStandAgents;

protected:
	//Virtual overrides
	virtual Entity::UpdateStatus frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);
	virtual bool isNonspatial();
};
}
#endif /* TAXISTANDAGENT_HPP_ */
