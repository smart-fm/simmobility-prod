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
	int min_start;
	int min_stop;
	bool started; // has the broker previously satisfied the min_start criteria?

public:
	WaitForAgentRegistration(sim_mob::Broker & broker_,int min_start = 1, int min_stop = 0);
	bool calculateWaitStatus();
	virtual ~WaitForAgentRegistration();
};

} /* namespace sim_mob */
