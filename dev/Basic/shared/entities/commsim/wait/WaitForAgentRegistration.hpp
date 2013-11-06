//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * WaitForAgentRegistration.hpp
 *
 *  Created on: Sep 9, 2013
 *      Author: vahid
 */

#pragma once

#include "BrokerBlocker.hpp"

namespace sim_mob {

class WaitForAgentRegistration: public sim_mob::BrokerBlocker {
	unsigned int min_start;
	unsigned int stop_threshold; //nof agents dropping 'below' this threshold will cause the broker to block
	bool started; // has the broker previously satisfied the min_start criteria?

public:
	WaitForAgentRegistration(sim_mob::Broker & broker_,unsigned int min_start = 1, unsigned int stop_threshold = 0);
	bool calculateWaitStatus();
	virtual ~WaitForAgentRegistration();
};

} /* namespace sim_mob */
