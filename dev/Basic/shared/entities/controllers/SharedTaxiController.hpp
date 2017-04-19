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
		unsigned int freq = 0) : MobilityServiceController(mtxStrat, freq)
	{
	}

protected:
	virtual bool isCruising(Person* p) = 0;
	virtual const Node* getCurrentNode(Person* p) = 0;

private:
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	std::vector<MessageResult> assignVehiclesToRequests();
};
}
#endif /* SharedTaxiController_HPP_ */



