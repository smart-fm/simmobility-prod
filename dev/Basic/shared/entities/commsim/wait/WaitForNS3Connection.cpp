//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "WaitForNS3Connection.hpp"
#include "entities/commsim/broker/Broker.hpp"

using namespace sim_mob;

sim_mob::WaitForNS3Connection::WaitForNS3Connection() : BrokerBlocker(), numClients(0)
{
}

sim_mob::WaitForNS3Connection::~WaitForNS3Connection()
{
}

void sim_mob::WaitForNS3Connection::reset(unsigned int numClients)
{
	this->numClients = numClients;
	passed = false;
}

bool sim_mob::WaitForNS3Connection::calculateWaitStatus(BrokerBase& broker) const
{
	const ClientList::Type& clients = broker.getClientList();
	ClientList::Type::const_iterator it = clients.find(comm::NS3_SIMULATOR);
	if (it==clients.end()) {
		throw std::runtime_error("Unexpected in WaitForNS3Connection::calculateWaitStatus()");
	}

	return it->second.size()>=numClients;
}

