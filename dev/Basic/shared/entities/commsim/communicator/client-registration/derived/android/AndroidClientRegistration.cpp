/*
 * AndroidClientRegistration.cpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#include "AndroidClientRegistration.hpp"

namespace sim_mob {

AndroidClientRegistration::AndroidClientRegistration(ClientType type_) : ClientRegistrationHandler(type_){
	// TODO Auto-generated constructor stub

}

bool AndroidClientRegistration::handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest request)
{
	boost::unique_lock< boost::mutex > lock(*broker.getBrokerMutex());
	//some checks to avoid calling this expensive method unnecessarily
	if(
			broker.getClientWaitingList().empty()
			|| broker.getRegisteredAgents().empty()
			|| usedAgents.size() == broker.getRegisteredAgents().size()
			)
	{
		return false;
	}

	AgentsMap &registeredAgents = broker.getRegisteredAgents();
	//find a free agent
	 std::pair<sim_mob::Agent *, JCommunicationSupport* >* freeAgent;
	for(AgentsMap::iterator it = registeredAgents.begin(), it_end = registeredAgents.end() ; it != it_end; it++)
	{
		if(usedAgents.find(it->first) != usedAgents.end())
		{
			//this agent is already associated with a client whose type is handled by this class
			continue;
		}
		//found a free agent
		freeAgent = it;
		//use it to create a client entry
		registeredClientEntry clientEntry;
		clientEntry.cnnHandler.reset(new ConnectionHandler(
				request.session_
				,broker
				,request.clientID
				,(unsigned long)(freeAgent->first)//just remembered that we can/should filter agents based on the agent type ...-vahid
				)
		);

		clientEntry.JCommunicationSupport_ = freeAgent->second;
		//todo: some of there information are already available in the connectionHandler! omit redundancies  -vahid
		clientEntry.agent = freeAgent->first;
		clientEntry.clientID = request.clientID;
		clientEntry.client_type = myType;
		clientEntry.requiredCapabilities = request.requiredCapabilities;

		//add the client entry to broker
		broker.getClientList().insert(std::make_pair(myType,clientEntry));

		//add this agent to the list of the agents who are associated with a android emulator client
		usedAgents.insert( *it);
		//we have nothing to do in this method
		return true;
	}
	//you couldn't find a free function
	return false;
}
AndroidClientRegistration::~AndroidClientRegistration() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
