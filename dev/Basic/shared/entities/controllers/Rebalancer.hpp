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

class OnCallController; //forward declaration

class Rebalancer
{
public:
	Rebalancer(const OnCallController* parentController_);
	virtual ~Rebalancer();

	virtual void rebalance(const std::vector<const Person*>& availableDrivers,
			const timeslice currTick)=0;

	void onRequestReceived(const Node* startNode);

protected:
	std::vector<const Node*> latestStartNodes;
	const OnCallController* parentController;
};

class SimpleRebalancer : public Rebalancer
{
	using Rebalancer::Rebalancer;
	//SimpleRebalancer(const OnCallController* parentController_):Rebalancer(parentController_);

	void rebalance(const std::vector<const Person*>& availableDrivers,
			const timeslice currTick);
};


} /* namespace sim_mob */

