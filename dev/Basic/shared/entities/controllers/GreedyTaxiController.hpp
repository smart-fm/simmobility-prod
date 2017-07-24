/*
 * GreedyTaxiController.hpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef GreedyTaxiController_HPP_
#define GreedyTaxiController_HPP_
#include <vector>

#include "entities/Agent.hpp"
#include "OnCallController.hpp"

namespace sim_mob
{

class GreedyTaxiController : public OnCallController {
public:
	GreedyTaxiController
		(const MutexStrategy& mtxStrat, unsigned int computationPeriod, unsigned id, TT_EstimateType ttEstimateType) :
		OnCallController(mtxStrat, computationPeriod, MobilityServiceControllerType::SERVICE_CONTROLLER_GREEDY, id, ttEstimateType)
	{
	}


protected:
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual void computeSchedules();
};
}
#endif /* GreedyTaxiController_HPP_ */
