/*
 * WaitForNS3Connection.hpp
 *
 *  Created on: Jul 15, 2013
 *      Author: vahid
 */

#ifndef WAITFORNS3CONNECTION_HPP_
#define WAITFORNS3CONNECTION_HPP_

#include "WaitForClientConnection.hpp"

namespace sim_mob {
class Broker;

class WaitForNS3Connection: public sim_mob::WaitForClientConnection {
	int min_nof_clients;
public:
	WaitForNS3Connection(sim_mob::Broker & broker_,int min_nof_clients_ = 1);
	short get_MIN_NOF_Clients();
	void set_MIN_NOF_Clients(int);
	bool evaluate();
	virtual ~WaitForNS3Connection();
};

} /* namespace sim_mob */
#endif /* WAITFORNS3CONNECTION_HPP_ */
