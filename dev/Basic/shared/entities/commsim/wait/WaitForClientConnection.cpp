/*
 * WaitForClientConnection.cpp
 *
 *  Created on: Jul 15, 2013
 *      Author: vahid
 */

#include "WaitForClientConnection.hpp"
#include "entities/commsim/broker/Broker.hpp"

namespace sim_mob {

WaitForClientConnection::WaitForClientConnection(sim_mob::Broker & broker_): broker(broker_) {
	// TODO Auto-generated constructor stub
	wait_status = true;
}

sim_mob::Broker & WaitForClientConnection::getBroker() const{
	return broker;
}

bool WaitForClientConnection::isWaiting() {
	boost::unique_lock<boost::mutex> lock(mutex_);
	return wait_status;
}

void WaitForClientConnection::setWaitStatus(bool value) {
	boost::unique_lock<boost::mutex> lock(mutex_);
	wait_status = value;
}

WaitForClientConnection::~WaitForClientConnection() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
