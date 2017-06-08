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
#include "entities/controllers/MobilityServiceController.hpp"

namespace sim_mob
{

class SharedController : public MobilityServiceController {
public:
	explicit SharedController(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered,
		unsigned int computationPeriod = 0) : MobilityServiceController(mtxStrat, computationPeriod)
	{
	}

protected:
	bool isCruising(Person* p);
	const Node* getCurrentNode(Person* p);

	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	void computeSchedules();
};
}
#endif /* SharedController_HPP_ */
