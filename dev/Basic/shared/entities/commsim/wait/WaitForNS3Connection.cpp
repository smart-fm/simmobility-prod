/*
 * WaitForNS3Connection.cpp
 *
 *  Created on: Jul 15, 2013
 *      Author: vahid
 */

#include "WaitForNS3Connection.hpp"
#include "entities/commsim/broker/Broker.hpp"
#include <boost/unordered_map.hpp>

namespace sim_mob {

WaitForNS3Connection::WaitForNS3Connection(sim_mob::Broker & broker_,int min_nof_clients_ ):
				BrokerBlocker(broker_),
				min_nof_clients(min_nof_clients_) {
	// TODO Auto-generated constructor stub

}

short WaitForNS3Connection::get_MIN_NOF_Clients() {
	return min_nof_clients;
}

void WaitForNS3Connection::set_MIN_NOF_Clients(int value) {
	min_nof_clients = value;
}

bool WaitForNS3Connection::calculateWaitStatus() {
	ClientList::type & clients = getBroker().getClientList();
	int cnt = clients[ConfigParams::NS3_SIMULATOR].size();
	if(cnt >= min_nof_clients)
	{
		setWaitStatus(false);
	}
	else {
	setWaitStatus(true);
	}
	return isWaiting();//need to wait

}

WaitForNS3Connection::~WaitForNS3Connection() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */