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
	public:	ProximityBased(const MutexStrategy &mtxStrat, unsigned int computationPeriod, unsigned id,
            TT_EstimateType ttEstimateType) : OnCallController(mtxStrat, computationPeriod,
                                                               MobilityServiceControllerType::SERVICE_CONTROLLER_PROXIMITY,
                                                               id, ttEstimateType)
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
