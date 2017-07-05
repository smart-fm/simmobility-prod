/*
 * SharedController.hpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef SharedController_HPP_
#define SharedController_HPP_
#include <vector>

#include "entities/Agent.hpp"
#include "OnCallController.hpp"

namespace sim_mob
{

class SharedController : public OnCallController {
public:
	SharedController
		(const MutexStrategy& mtxStrat, unsigned int computationPeriod, unsigned id) :
		OnCallController(mtxStrat, computationPeriod, MobilityServiceControllerType::SERVICE_CONTROLLER_SHARED, id)
	{
	}

	virtual void checkSequence (const std::string& sequence) const;


protected:
	virtual bool isCruising(Person* p);
	virtual const Node* getCurrentNode(Person* p);

	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual void computeSchedules();
};
}
#endif /* SharedController_HPP_ */
