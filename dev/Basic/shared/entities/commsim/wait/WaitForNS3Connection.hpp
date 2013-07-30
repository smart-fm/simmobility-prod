/*
 * WaitForNS3Connection.hpp
 *
 *  Created on: Jul 15, 2013
 *      Author: vahid
 */

#pragma once

#include "WaitForClientConnection.hpp"

namespace sim_mob {
class Broker;

class WaitForNS3Connection: public sim_mob::WaitForClientConnection {
	int min_nof_clients;
public:
	WaitForNS3Connection(sim_mob::Broker & broker_,int min_nof_clients_ = 1);
	short get_MIN_NOF_Clients();
	void set_MIN_NOF_Clients(int);
	bool calculateWaitStatus();
	virtual ~WaitForNS3Connection();
};

} /* namespace sim_mob */

