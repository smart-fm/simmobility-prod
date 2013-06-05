/*
 * AndroidClientRegistration.cpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#include "AndroidClientRegistration.hpp"
#include "entities/commsim/communicator/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/communicator/connection/ConnectionHandler.hpp"
#include "entities/commsim/communicator/service/base/Publisher.hpp"
#include "entities/commsim/communicator/broker/Common.hpp"
#include "entities/commsim/comm_support/JCommunicationSupport.hpp"
namespace sim_mob {

AndroidClientRegistration::AndroidClientRegistration(/*ClientType type_) : ClientRegistrationHandler(type_*/){
	// TODO Auto-generated constructor stub

}

bool AndroidClientRegistration::handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest request)
{
//	boost::unique_lock< boost::mutex > lock(*broker.getBrokerClientMutex());
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
//	 std::pair<const sim_mob::Agent *, JCommunicationSupport<std::string> * >* freeAgent = 0;
	 AgentsMap::iterator freeAgent = registeredAgents.begin(), it_end = registeredAgents.end();
	for(  ; freeAgent != it_end; freeAgent++)
	{
		if(usedAgents.find(freeAgent->first) == usedAgents.end())
		{
			//found a free agent
			break;
		}
	}
	if(freeAgent == it_end)
	{
		//you couldn't find a free function
		return false;
	}

		//use it to create a client entry
		boost::shared_ptr<ClientHandler> clientEntry(new ClientHandler(broker));
		clientEntry->cnnHandler.reset(new ConnectionHandler(
				request.session_
				,broker
				,&Broker::messageReceiveCallback
				,request.clientID
				,(unsigned long int)(freeAgent->first)//just remembered that we can/should filter agents based on the agent type ...-vahid
				)

		);

		clientEntry->JCommunicationSupport_ = freeAgent->second;
		//todo: some of there information are already available in the connectionHandler! omit redundancies  -vahid
		clientEntry->agent = freeAgent->first;
		clientEntry->clientID = request.clientID;
		clientEntry->client_type = ANDROID_EMULATOR;
		clientEntry->requiredServices = request.requiredServices; //will come handy
		SIM_MOB_SERVICE srv;
		BOOST_FOREACH(srv, request.requiredServices)
		{
			switch(srv)
			{
			case SIMMOB_SRV_TIME:{
				 boost::shared_ptr<sim_mob::Publisher> p = broker.getPublishers()[SIMMOB_SRV_TIME];
				p->Subscribe(COMMEID_TIME, clientEntry.get(), CALLBACK_HANDLER(sim_mob::TimeEventArgs, ClientHandler::OnTime) );
				break;
			}
			case SIMMOB_SRV_LOCATION:{
				 boost::shared_ptr<sim_mob::Publisher> p = broker.getPublishers()[SIMMOB_SRV_LOCATION];
				p->Subscribe(COMMEID_LOCATION,(void*) clientEntry->agent, clientEntry.get()  ,CONTEXT_CALLBACK_HANDLER(LocationEventArgs, ClientHandler::OnLocation) );
				break;
			}
			}
		}

		//also, add the client entry to broker(for message handler purposes)
		broker.getClientList().insert(std::make_pair(ANDROID_EMULATOR,clientEntry));

		//start listening to the handler
		clientEntry->cnnHandler->start();

		//add this agent to the list of the agents who are associated with a android emulator client
		usedAgents.insert( *freeAgent);
		//tell the agent you are registered
//		Print() << "AndroidClientRegistration::handle=>Broker Mutexes : " << broker.getBrokerMutexCollection()[0] << " " << broker.getBrokerMutexCollection()[1] << " " << broker.getBrokerMutexCollection()[2] << std::endl;
		std::vector<boost::shared_ptr<boost::shared_mutex > > & mutexes = broker.getBrokerMutexCollection();
//		Print() << "AndroidClientRegistration::handle=>       mutexes : " << broker.getBrokerMutexCollection()[0] << " " << broker.getBrokerMutexCollection()[1] << " " << broker.getBrokerMutexCollection()[2] << std::endl;

		freeAgent->second->setregistered(true);
		Print() << "AndroidClientRegistration::handle=> client associated to agent " << freeAgent->first << std::endl;
		return true;
}
AndroidClientRegistration::~AndroidClientRegistration() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
