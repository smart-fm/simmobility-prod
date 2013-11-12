//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * WaitForAndroidConnection.hpp
 *
 *  Created on: Jul 15, 2013
 *      Author: vahid
 */

#pragma once

#include "BrokerBlocker.hpp"

namespace sim_mob {

class WaitForAndroidConnection: public sim_mob::BrokerBlocker {
	int min_nof_clients;
public:
	WaitForAndroidConnection(sim_mob::Broker &,int min_nof_clients_ = 1);
	short get_MIN_NOF_Clients();
	void set_MIN_NOF_Clients(int);
	bool calculateWaitStatus();
	virtual ~WaitForAndroidConnection();
};

} /* namespace sim_mob */

