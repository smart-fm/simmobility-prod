//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BrokerBlocker.hpp"

#include "entities/commsim/broker/Broker.hpp"

using namespace sim_mob;

sim_mob::BrokerBlocker::BrokerBlocker() : passed(false)
{
}

sim_mob::BrokerBlocker::~BrokerBlocker()
{
}


bool sim_mob::BrokerBlocker::pass(BrokerBase& broker)
{
	if (!passed) {
		passed = calculateWaitStatus(broker);
	}
	return passed;
}

