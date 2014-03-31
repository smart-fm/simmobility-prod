//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "WaitForAgentRegistration.hpp"
#include "entities/commsim/Broker.hpp"

using namespace sim_mob;

sim_mob::WaitForAgentRegistration::WaitForAgentRegistration(sim_mob::Broker& broker, unsigned int minAgents) :
	BrokerBlocker(broker), minAgents(minAgents),  started(false)
{
}

sim_mob::WaitForAgentRegistration::~WaitForAgentRegistration()
{
}


bool WaitForAgentRegistration::calculateWaitStatus()
{
	size_t numAg = getBroker().getRegisteredAgentsSize();
	//AgentsList::type& registeredAgents = getBroker().getRegisteredAgents();
	//int size = registeredAgents.size();
	if(!started) {
		bool waitStat = numAg<minAgents;
		setWaitStatus(waitStat);
		if (waitStat) {
			started = true;
		}

		/*if(size >= minAgents) {
			//no need to wait
			setWaitStatus(false);
			//you are considered as "started"
			started = true;
		} else {
			Print() << "min_start(" <<  min_start << ") >= registeredAgents.size(" << size << ") =>setWaitStatus(true)" << std::endl;
			setWaitStatus(true);
		}*/
	} else {
		setWaitStatus(false);
		/*if(size < 0) { //Size will never be <0
			Print() << "min_stop=>registeredAgents.size()= " << size << std::endl;
			//you must wait
			setWaitStatus(true);
		} else {
			setWaitStatus(false);
		}*/
	}

	return isWaiting();
}

