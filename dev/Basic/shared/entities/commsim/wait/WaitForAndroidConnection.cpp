//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "WaitForAndroidConnection.hpp"
#include "entities/commsim/broker/Broker.hpp"

#include "logging/Log.hpp"

using namespace sim_mob;

sim_mob::WaitForAndroidConnection::WaitForAndroidConnection() : BrokerBlocker(), numClients(0)
{
}

sim_mob::WaitForAndroidConnection::~WaitForAndroidConnection()
{
}

void sim_mob::WaitForAndroidConnection::reset(unsigned int numClients)
{
	this->numClients = numClients;
	passed = false;
}

bool sim_mob::WaitForAndroidConnection::calculateWaitStatus(BrokerBase& broker) const
{
	Print() <<"Clients connected: " <<broker.getAndroidClientList().size() <<" of " <<numClients <<"\n";
	return broker.getAndroidClientList().size()>=numClients;
}

