/*
 * OnHailTaxiController.hpp
 *
 *  Created on: May 22, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef OnHailTaxiController_HPP_
#define OnHailTaxiController_HPP_
#include <vector>

#include "entities/Agent.hpp"
#include "entities/controllers/MobilityServiceController.hpp"

namespace sim_mob
{

class OnHailTaxiController : public MobilityServiceController {
public:
	explicit OnHailTaxiController(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered,
		unsigned int computationPeriod = 0) : MobilityServiceController(mtxStrat, computationPeriod)
	{
	}

protected:
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	void computeSchedules();
};
}
#endif /* OnHailTaxiController_HPP_ */
