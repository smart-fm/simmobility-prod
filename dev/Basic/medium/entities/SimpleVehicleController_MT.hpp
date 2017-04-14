/*
 * SimpleVehicleController_MT.hpp
 *
 *  Created on: Apr 13, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef SimpleVehicleController_MT_HPP_
#define SimpleVehicleController_MT_HPP_
#include <vector>

#include "entities/Agent.hpp"
#include "entities/controllers/VehicleController.hpp"

namespace sim_mob
{

class SimpleVehicleController_MT : public VehicleController {
public:
	explicit SimpleVehicleController_MT(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered,
		unsigned int freq = 0) : VehicleController(mtxStrat, freq)
	{
	}

private:
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	std::vector<MessageResult> assignVehiclesToRequests();
};
}
#endif /* SimpleVehicleController_MT_HPP_ */


