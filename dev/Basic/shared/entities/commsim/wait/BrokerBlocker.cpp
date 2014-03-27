//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BrokerBlocker.hpp"
#include "entities/commsim/Broker.hpp"

using namespace sim_mob;

sim_mob::BrokerBlocker::BrokerBlocker(sim_mob::Broker & broker_) : broker(broker_)
{
	wait_status = true;
}

sim_mob::BrokerBlocker::~BrokerBlocker()
{
}

sim_mob::Broker& sim_mob::BrokerBlocker::getBroker() const
{
	return broker;
}

bool sim_mob::BrokerBlocker::isWaiting() const
{
	boost::unique_lock<boost::mutex> lock(mutex_);
	return wait_status;
}

void sim_mob::BrokerBlocker::setWaitStatus(bool value)
{
	boost::unique_lock<boost::mutex> lock(mutex_);
	wait_status = value;
}

