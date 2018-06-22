/*
 * GreedyController.hpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#pragma once

#include <vector>

#include "entities/Agent.hpp"
#include "OnCallController.hpp"

namespace sim_mob
{

class GreedyController : public OnCallController
{
public:
	GreedyController
			(const MutexStrategy &mtxStrat, unsigned int computationPeriod, unsigned id, std::string tripSupportMode_ ,TT_EstimateType ttEstimateType,
             unsigned maxAggregatedRequests_, bool studyAreaEnabledController_,unsigned int toleratedExtraTime_,unsigned int maxWaitingTime_,bool parkingEnabled)
			:
			OnCallController(mtxStrat, computationPeriod, MobilityServiceControllerType::SERVICE_CONTROLLER_GREEDY, id, tripSupportMode_,
			                 ttEstimateType,maxAggregatedRequests_,studyAreaEnabledController_,toleratedExtraTime_,maxWaitingTime_,parkingEnabled)
	{
	}


protected:
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual void computeSchedules();
};
}
