/*
 * ProximityBased.h
 *
 *  Created on: Aug 8, 2017
 *      Author: araldo
 */

#ifndef ENTITIES_CONTROLLERS_PROXIMITYBASED_HPP_
#define ENTITIES_CONTROLLERS_PROXIMITYBASED_HPP_

#include "OnCallController.hpp"

namespace sim_mob {

class ProximityBased: public OnCallController
{
	public:	ProximityBased(const MutexStrategy &mtxStrat, unsigned int computationPeriod, unsigned id, std::string tripSupportMode_,
            TT_EstimateType ttEstimateType, unsigned maxAggregatedRequests_) : OnCallController(mtxStrat, computationPeriod,
                                                               MobilityServiceControllerType::SERVICE_CONTROLLER_PROXIMITY,
                                                               id,  tripSupportMode_,ttEstimateType,maxAggregatedRequests_)
	{}

		virtual ~ProximityBased(){};

	protected:
		/**
		 * Performs the controller algorithm to assign vehicles to requests
		 */
		virtual void computeSchedules();
};

} /* namespace sim_mob */

#endif /* ENTITIES_CONTROLLERS_PROXIMITYBASED_HPP_ */
