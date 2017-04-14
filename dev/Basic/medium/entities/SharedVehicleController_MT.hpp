/*
 * SharedVehicleController_MT.hpp
 *
 *  Created on: Apr 13, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef SharedVehicleController_MT_HPP_
#define SharedVehicleController_MT_HPP_
#include <vector>

#include "entities/Agent.hpp"
#include "entities/controllers/VehicleController.hpp"

namespace sim_mob
{

class SharedVehicleController_MT : public VehicleController {
public:
	explicit SharedVehicleController_MT(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered,
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
#endif /* SharedVehicleController_MT_HPP_ */


