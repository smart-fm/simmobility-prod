/*
 * SharedTaxiController.hpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef SharedTaxiController_HPP_
#define SharedTaxiController_HPP_
#include <vector>

#include "entities/Agent.hpp"
#include "entities/controllers/MobilityServiceController.hpp"

namespace sim_mob
{

class SharedTaxiController : public MobilityServiceController {
public:
	explicit SharedTaxiController(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered,
		unsigned int computationPeriod = 0) : MobilityServiceController(mtxStrat, computationPeriod)
	{
	}

protected:
	bool isCruising(Person* p);
	const Node* getCurrentNode(Person* p);

private:
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	std::vector<MessageResult> computeSchedules();
};
}
#endif /* SharedTaxiController_HPP_ */




