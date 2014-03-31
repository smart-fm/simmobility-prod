//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "BrokerBlocker.hpp"

namespace sim_mob {
class Broker;

class WaitForNS3Connection: public sim_mob::BrokerBlocker {
public:
	WaitForNS3Connection(sim_mob::Broker & broker_,int min_nof_clients_ = 1);
	virtual ~WaitForNS3Connection();

	short get_MIN_NOF_Clients();
	void set_MIN_NOF_Clients(int);
	virtual bool calculateWaitStatus();

private:
	int min_nof_clients;
};

}
