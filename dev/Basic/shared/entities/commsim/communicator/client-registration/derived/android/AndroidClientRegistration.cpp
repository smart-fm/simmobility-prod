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

AndroidClientRegistration::AndroidClientRegistration(/*ConfigParams::ClientType type_) : ClientRegistrationHandler(type_*/){
	// TODO Auto-generated constructor stub

}

bool AndroidClientRegistration::handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest request)
{
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
		boost::shared_ptr<sim_mob::ConnectionHandler > cnnHandler(new ConnectionHandler(
				request.session_
				,broker
				,&Broker::messageReceiveCallback
				,request.clientID
				,ConfigParams::ANDROID_EMULATOR
				,(unsigned long int)(freeAgent->first)//just remembered that we can/should filter agents based on the agent type ...-vahid
				)
				);
		clientEntry->cnnHandler = cnnHandler;

//		Print()<< "Connection handler for agent "

		clientEntry->JCommunicationSupport_ = freeAgent->second;
		//todo: some of there information are already available in the connectionHandler! omit redundancies  -vahid
		clientEntry->agent = freeAgent->first;
		clientEntry->clientID = request.clientID;
		clientEntry->client_type = ConfigParams::ANDROID_EMULATOR;
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
		broker.insertClientList(clientEntry->clientID, ConfigParams::ANDROID_EMULATOR,clientEntry);
//		Print() << "clientEntry[" << clientEntry << "].use_count(" << clientEntry.use_count() << ")" << std::endl;
//		Print() << "clientEntry.cnnhandler[" << clientEntry->cnnHandler << "].use_count(" << clientEntry->cnnHandler.use_count() << ")" << std::endl;

		//add this agent to the list of the agents who are associated with a android emulator client
		usedAgents.insert( *freeAgent);
		//tell the agent you are registered
		freeAgent->second->setregistered(true);
		Print() << "AndroidClientRegistration::handle=> client associated to agent " << freeAgent->first << std::endl;

		//start listening to the handler
		clientEntry->cnnHandler->start();
		return true;
}
AndroidClientRegistration::~AndroidClientRegistration() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
