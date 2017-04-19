/*
 * GreedyTaxiController_MT.hpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef GreedyTaxiController_MT_HPP_
#define GreedyTaxiController_MT_HPP_
#include <vector>

#include "entities/Agent.hpp"
#include "entities/controllers/GreedyTaxiController.hpp"

namespace sim_mob
{

class GreedyTaxiController_MT : public GreedyTaxiController {
public:
	explicit GreedyTaxiController_MT(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered,
		unsigned int freq = 0) : GreedyTaxiController(mtxStrat, freq)
	{
	}
protected:
	bool isCruising(Person* p);
	const Node* getCurrentNode(Person* p);
};
}
#endif /* GreedyTaxiController_MT_HPP_ */



