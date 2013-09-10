/*
 * WaitForAgentRegistration.cpp
 *
 *  Created on: Sep 9, 2013
 *      Author: vahid
 */

#include "WaitForAgentRegistration.hpp"
#include "entities/commsim/broker/Broker.hpp"
namespace sim_mob {

WaitForAgentRegistration::WaitForAgentRegistration(sim_mob::Broker & broker_,int min_start, int min_stop):
				BrokerBlocker(broker_),
				min_start(min_start),
				min_stop(min_stop),
				started(false){
	// TODO Auto-generated constructor stub

}

bool WaitForAgentRegistration::calculateWaitStatus() {
	AgentsMap::type &registeredAgents = getBroker().getRegisteredAgents();
	if(!started)
	{
		if(registeredAgents.size() >= min_start)
		{
			//no need to wait
			setWaitStatus(false);
			//you are considered as "started"
			started = true;
		}
		else
		{
			setWaitStatus(true);
		}
	}
	else {//if already started
		if(registeredAgents.size() <= min_stop)
		{
			//you must wait
			setWaitStatus(true);
		}
		else
		{
			setWaitStatus(false);
		}

	}

	return isWaiting();
}

WaitForAgentRegistration::~WaitForAgentRegistration() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
