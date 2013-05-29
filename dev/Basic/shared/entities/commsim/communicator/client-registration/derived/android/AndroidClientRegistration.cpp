/*
 * AndroidClientRegistration.cpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#include "AndroidClientRegistration.hpp"

namespace sim_mob {

AndroidClientRegistration::AndroidClientRegistration(/*ClientType type_) : ClientRegistrationHandler(type_*/){
	// TODO Auto-generated constructor stub

}

bool AndroidClientRegistration::handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest request)
{
	boost::unique_lock< boost::mutex > lock(*broker.getBrokerMutex());
	//some checks to avoid calling this method unnecessarily
	if(
			broker.getClientWaitingList().empty()
			|| broker.getRegisteredAgents().empty()
			|| usedAgents.size() == broker.getRegisteredAgents().size()
			)
	{
		return false;
	}

	AgentsMap &registeredAgents = broker.getRegisteredAgents();
	//find a free agent(someone who is not yet been associated to an andriod client)
	 std::pair<sim_mob::Agent *, JCommunicationSupport* >* freeAgent = 0;
	for(AgentsMap::iterator it = registeredAgents.begin(), it_end = registeredAgents.end() ; it != it_end; it++)
	{
		if(usedAgents.find(it->first) == usedAgents.end())
		{
			//found a free agent
			freeAgent = it;
			break;
		}
	}
	if(freeAgent == 0)
	{
		//you couldn't find a free function
		return false;
	}
		//use it to create a client entry
		boost::shared_ptr<AndroidClientHandler> clientEntry(new ClientHandler(broker));
		clientEntry->cnnHandler.reset(new ConnectionHandler(
				request.session_
				,broker
				,request.clientID
				,(unsigned long)(freeAgent->first)//just remembered that we can/should filter agents based on the agent type ...-vahid
				)

		);

		clientEntry->JCommunicationSupport_ = freeAgent->second;
		//todo: some of there information are already available in the connectionHandler! omit redundancies  -vahid
		clientEntry->agent = freeAgent->first;
		clientEntry->clientID = request.clientID;
		clientEntry->client_type = myType;
		clientEntry->requiredServices = request.requiredServices; //will come handy
		SIM_MOB_SERVICE srv;
		BOOST_FOREACH(srv, request.requiredServices)
		{
			switch(srv)
			{
			case SIMMOB_SRV_TIME:
				broker.getPublishers()[SIMMOB_SRV_TIME]->Subscribe(COMMEID_TIME, clientEntry, CALLBACK_HANDLER(sim_mob::TimeEventArgs, ClientHandler::OnTime) );
				break;
			case SIMMOB_SRV_LOCATION:
				broker.getPublishers()[SIMMOB_SRV_LOCATION]->Suscribe(COMMEID_LOCATION, clientEntry->agent, clientEntry  ,CONTEXT_CALLBACK_HANDLER(LocationEventArgs, ClientHandler::OnLocation) );
				break;
			}
		}

		//also, add the client entry to broker(for message handler purposes)
		broker.getClientList().insert(std::make_pair(myType,clientEntry));

		//start listening to the handler
		clientEntry->cnnHandler->start();

		//add this agent to the list of the agents who are associated with a android emulator client
		usedAgents.insert( *freeAgent);

		return true;
}
AndroidClientRegistration::~AndroidClientRegistration() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
