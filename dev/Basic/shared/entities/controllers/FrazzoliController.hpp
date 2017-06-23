/*
 * FrazzoliController.h
 *
 *  Created on: 21 Jun 2017
 *      Author: araldo
 */

#ifndef SHARED_ENTITIES_CONTROLLERS_FRAZZOLICONTROLLER_HPP_
#define SHARED_ENTITIES_CONTROLLERS_FRAZZOLICONTROLLER_HPP_

#include "OnCallController.hpp"

namespace sim_mob {

class FrazzoliController: public OnCallController {
public:
	explicit FrazzoliController
		(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered,
		unsigned int computationPeriod = 0) :
		OnCallController(mtxStrat, computationPeriod, MobilityServiceControllerType::SERVICE_CONTROLLER_FRAZZOLI)
	{
	}

	virtual ~FrazzoliController();

	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual void computeSchedules();
};

} /* namespace sim_mob */

#endif /* SHARED_ENTITIES_CONTROLLERS_FRAZZOLICONTROLLER_HPP_ */
