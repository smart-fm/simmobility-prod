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
#include "entities/controllers/MobilityServiceController.hpp"

namespace sim_mob
{

class GreedyTaxiController : public MobilityServiceController {
public:
	explicit GreedyTaxiController(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered,
		unsigned int computationPeriod = 0) : MobilityServiceController(mtxStrat, computationPeriod)
	{
	}

protected:
	virtual bool isCruising(Person* p) = 0;
	virtual const Node* getCurrentNode(Person* p) = 0;

private:
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	std::vector<MessageResult> computeSchedules();
};
}
#endif /* GreedyTaxiController_HPP_ */




