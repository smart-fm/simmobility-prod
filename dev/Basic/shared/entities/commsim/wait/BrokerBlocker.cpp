//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * BrokerBlocker.cpp
 *
 *  Created on: Jul 15, 2013
 *      Author: vahid
 */

#include "BrokerBlocker.hpp"
#include "entities/commsim/Broker.hpp"

namespace sim_mob {

BrokerBlocker::BrokerBlocker(sim_mob::Broker & broker_): broker(broker_) {
	// TODO Auto-generated constructor stub
	wait_status = true;
}

sim_mob::Broker & BrokerBlocker::getBroker() const{
	return broker;
}

bool BrokerBlocker::isWaiting() {
	boost::unique_lock<boost::mutex> lock(mutex_);
	return wait_status;
}

void BrokerBlocker::setWaitStatus(bool value) {
	boost::unique_lock<boost::mutex> lock(mutex_);
	wait_status = value;
}

BrokerBlocker::~BrokerBlocker() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
