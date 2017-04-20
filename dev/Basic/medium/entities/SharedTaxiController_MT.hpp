/*
 * SharedTaxiController_MT.hpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef SharedTaxiController_MT_HPP_
#define SharedTaxiController_MT_HPP_
#include <vector>

#include "entities/Agent.hpp"
#include "entities/controllers/SharedTaxiController.hpp"

namespace sim_mob
{

class SharedTaxiController_MT : public SharedTaxiController {
public:
	explicit SharedTaxiController_MT(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered,
		unsigned int computationPeriod = 0) : SharedTaxiController(mtxStrat, computationPeriod)
	{
	}
	
protected:
	bool isCruising(Person* p);
	const Node* getCurrentNode(Person* p);
};
}
#endif /* SharedTaxiController_MT_HPP_ */




