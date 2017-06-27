/*
 * Rebalancer.hpp
 *
 *  Created on: Jun 1, 2017
 *      Author: araldo
 */
#pragma once
#include <queue>
#include "entities/Person.hpp"

namespace sim_mob {

class Rebalancer {
public:
	Rebalancer();
	virtual ~Rebalancer();

	virtual void rebalance(const std::vector<const Person*>& availableDrivers,
			const timeslice currTick)=0;

	void sendCruiseTAZ_Command(const Person* driver, unsigned tazId, const timeslice currTick ) const;

	void onRequestReceived(const Node* startNode);

protected:
	std::vector<const Node*> latestStartNodes;
};

class SimpleRebalancer : public Rebalancer
{
	void rebalance(const std::vector<const Person*>& availableDrivers,
			const timeslice currTick);
};

class KasiaRebalancer : public Rebalancer
{
	void rebalance(const std::vector<const Person*>& availableDrivers,
			const timeslice currTick);
};


} /* namespace sim_mob */

