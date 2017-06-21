/*
 * FrazzoliController.h
 *
 *  Created on: 21 Jun 2017
 *      Author: araldo
 */

#ifndef SHARED_ENTITIES_CONTROLLERS_FRAZZOLICONTROLLER_H_
#define SHARED_ENTITIES_CONTROLLERS_FRAZZOLICONTROLLER_H_

#include "OnCallController.hpp"

namespace sim_mob {

class FrazzoliController: public OnCallController {
public:
	FrazzoliController();
	virtual ~FrazzoliController();

	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual void computeSchedules();
};

} /* namespace sim_mob */

#endif /* SHARED_ENTITIES_CONTROLLERS_FRAZZOLICONTROLLER_H_ */
