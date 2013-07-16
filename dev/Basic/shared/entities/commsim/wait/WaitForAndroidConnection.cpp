/*
 * WaitForAndroidConnection.cpp
 *
 *  Created on: Jul 15, 2013
 *      Author: vahid
 */

#include "WaitForAndroidConnection.hpp"
#include "entities/commsim/broker/Broker.hpp"
#include <boost/unordered_map.hpp>

namespace sim_mob {

WaitForAndroidConnection::WaitForAndroidConnection(sim_mob::Broker & broker_,int min_nof_clients_ ):
		WaitForClientConnection(broker_),
		min_nof_clients(min_nof_clients_) {
	// TODO Auto-generated constructor stub

}

short WaitForAndroidConnection::get_MIN_NOF_Clients() {
	return min_nof_clients;
}

void WaitForAndroidConnection::set_MIN_NOF_Clients(int value) {
	min_nof_clients = value;
}

bool WaitForAndroidConnection::evaluate() {
	Print() << "inside WaitForAndroidConnection::evaluate" << std::endl;
	ClientList::type & clients = getBroker().getClientList();
	int cnt = clients[ConfigParams::ANDROID_EMULATOR].size();
	if(cnt >= min_nof_clients)
	{
		Print() << "inside WaitForAndroidConnection--> dont wait" << std::endl;
		return false;//no need to wait
	}
	Print() << "inside WaitForAndroidConnection::evaluate()--> wait" << std::endl;
	return true;//need to wait
}

WaitForAndroidConnection::~WaitForAndroidConnection() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
