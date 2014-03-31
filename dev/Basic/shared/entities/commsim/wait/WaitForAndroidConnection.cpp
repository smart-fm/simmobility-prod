//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "WaitForAndroidConnection.hpp"
#include "entities/commsim/Broker.hpp"
#include <boost/unordered_map.hpp>

using namespace sim_mob;

sim_mob::WaitForAndroidConnection::WaitForAndroidConnection(sim_mob::Broker & broker_,int min_nof_clients_ ) :
	BrokerBlocker(broker_), min_nof_clients(min_nof_clients_)
{
}

sim_mob::WaitForAndroidConnection::~WaitForAndroidConnection()
{
}

short sim_mob::WaitForAndroidConnection::get_MIN_NOF_Clients()
{
	return min_nof_clients;
}

void sim_mob::WaitForAndroidConnection::set_MIN_NOF_Clients(int value)
{
	min_nof_clients = value;
}

bool sim_mob::WaitForAndroidConnection::calculateWaitStatus()
{
	const ClientList::Type & clients = getBroker().getClientList();
	ClientList::Type::const_iterator it = clients.find(comm::ANDROID_EMULATOR);
	if (it==clients.end()) {
		throw std::runtime_error("Unexpected in WaitForAndroidConnection::calculateWaitStatus()");
	}

	int cnt = it->second.size();
	if(cnt >= min_nof_clients) {
		setWaitStatus(false);
	} else {
		setWaitStatus(true);
	}
	return isWaiting();
}

