//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "WaitForAndroidConnection.hpp"
#include "entities/commsim/Broker.hpp"
#include <boost/unordered_map.hpp>

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
	const ClientList::Type & clients = broker.getClientList();
	ClientList::Type::const_iterator it = clients.find(comm::ANDROID_EMULATOR);
	if (it==clients.end()) {
		throw std::runtime_error("Unexpected in WaitForAndroidConnection::calculateWaitStatus()");
	}

	return it->second.size()>=numClients;
}

