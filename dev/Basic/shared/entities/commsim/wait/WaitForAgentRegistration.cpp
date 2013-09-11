/*
 * WaitForAgentRegistration.cpp
 *
 *  Created on: Sep 9, 2013
 *      Author: vahid
 */

#include "WaitForAgentRegistration.hpp"
#include "entities/commsim/broker/Broker.hpp"
namespace sim_mob {

WaitForAgentRegistration::WaitForAgentRegistration(sim_mob::Broker & broker_,unsigned int min_start, unsigned int stop_threshold):
				BrokerBlocker(broker_),
				min_start(min_start),
				stop_threshold(stop_threshold),
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
			Print() << "min_start=>registeredAgents.size()= " << registeredAgents.size() << std::endl;
			setWaitStatus(true);
		}
	}
	else {//if already started
		if(registeredAgents.size() < stop_threshold)
		{
			Print() << "min_stop=>registeredAgents.size()= " << registeredAgents.size() << std::endl;
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
