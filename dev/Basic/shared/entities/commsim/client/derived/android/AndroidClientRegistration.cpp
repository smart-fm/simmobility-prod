//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AndroidClientRegistration.cpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#include "AndroidClientRegistration.hpp"

#include "entities/commsim/broker/Broker.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "entities/commsim/broker/Common.hpp"
#include "entities/commsim/comm_support/AgentCommUtility.hpp"
#include "event/EventPublisher.hpp"
#include "entities/commsim/event/RegionsAndPathEventArgs.hpp"

using namespace sim_mob;

sim_mob::AndroidClientRegistration::AndroidClientRegistration() : ClientRegistrationHandler(comm::ANDROID_EMULATOR)
{
}

sim_mob::AndroidClientRegistration::~AndroidClientRegistration()
{
}

bool sim_mob::AndroidClientRegistration::handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest request)
{
	//This part is locked in fear of registered agents' iterator invalidation in the middle of the process
	AgentsList::Mutex registered_agents_mutex;
	AgentsList::type &registeredAgents = broker.getRegisteredAgents(&registered_agents_mutex);
	AgentsList::Lock lock(registered_agents_mutex);

	//some checks to avoid calling this method unnecessarily
	if (broker.getClientWaitingList().empty()
			|| registeredAgents.empty()
			|| usedAgents.size() == registeredAgents.size())
	{
		Print() << "AndroidClientRegistration::handle initial failure, returning false"
				<< broker.getClientWaitingList().size() << "-"
				<< registeredAgents.size() << "-"
				<< usedAgents.size() << std::endl;
		return false;
	}

	//find the first free agent(someone who is not yet been associated to an andriod client)
	bool found_a_free_agent = false;
	AgentsList::type::iterator freeAgent = registeredAgents.begin(), it_end = registeredAgents.end();
	for (; freeAgent != it_end; freeAgent++) {
		if (usedAgents.find(freeAgent->second.agent) == usedAgents.end()) {
			Print() << "Agent[" << freeAgent->second.agent->getId() << "]["<< freeAgent->second.agent << "] is free to associate with" << std::endl;
			found_a_free_agent = true;
			//found the first free agent, no need to continue the loop
			break;
		}
	}

	if (!found_a_free_agent) {
		//you couldn't find a free function
		Print() << "AndroidClientRegistration::handle couldn't find a free agent among [" << registeredAgents.size() << "], returning false"
				<< std::endl;
		return false;
	}

	//use it to create a client entry
	boost::shared_ptr<ClientHandler> clientEntry(new ClientHandler(broker));
	boost::shared_ptr<sim_mob::ConnectionHandler> cnnHandler(
			new ConnectionHandler(request.session_,
					broker.getMessageReceiveCallBack(), request.clientID,
					comm::ANDROID_EMULATOR,
					(unsigned long int) (freeAgent->first)//just remembered that we can/should filter agents based on the agent type ...-vahid
					));
	clientEntry->cnnHandler = cnnHandler;

	clientEntry->AgentCommUtility_ = freeAgent->second.comm;
	//todo: some of there information are already available in the connectionHandler! omit redundancies  -vahid
	clientEntry->agent = freeAgent->first;
	clientEntry->clientID = request.clientID;
	clientEntry->client_type = comm::ANDROID_EMULATOR;
	clientEntry->requiredServices = request.requiredServices; //will come handy
	sim_mob::Services::SIM_MOB_SERVICE srv;

	bool regionSupportRequired = false;
	BOOST_FOREACH(srv, request.requiredServices) {
		switch (srv) {
			case sim_mob::Services::SIMMOB_SRV_TIME: {
				PublisherList::Value p =
						broker.getPublisher(sim_mob::Services::SIMMOB_SRV_TIME);
				p->subscribe(COMMEID_TIME, 
											 clientEntry.get(),
											 &ClientHandler::sendJsonToBroker);
				break;
			}
			case sim_mob::Services::SIMMOB_SRV_LOCATION: {
				PublisherList::Value p =
						broker.getPublisher(sim_mob::Services::SIMMOB_SRV_LOCATION);

				//NOTE: It does not seem like we even use the "Context" pointer, so I am switching
				//      this to a regular CALLBACK_HANDLER. Please review. ~Seth
				p->subscribe(COMMEID_LOCATION, 
						clientEntry.get(),
						&ClientHandler::sendJsonToBroker);
				break;
			}
			case sim_mob::Services::SIMMOB_SRV_REGIONS_AND_PATH: {
				PublisherList::Value p = broker.getPublisher(sim_mob::Services::SIMMOB_SRV_REGIONS_AND_PATH);
				p->subscribe(COMMEID_REGIONS_AND_PATH, clientEntry.get(), &ClientHandler::sendJsonToBroker);

				//We also "enable" Region tracking for this Agent.
				regionSupportRequired = true;
				break;
			}
			default: {
				Warn() <<"Android client requested service which could not be provided.\n";
				break;
			}
		}
	}

	//Enable Region support if this client requested it.
	if (regionSupportRequired) {
		broker.pendClientToEnableRegions(clientEntry);
	}

	//also, add the client entry to broker(for message handler purposes)
	broker.insertClientList(clientEntry->clientID,
			comm::ANDROID_EMULATOR, clientEntry);
	//add this agent to the list of the agents who are associated with a android emulator client
	usedAgents.insert(freeAgent->second.agent);
	//tell the agent you are registered
	freeAgent->second.comm->setregistered(true);
	//publish an event to inform- interested parties- of the registration of a new android client
	getPublisher().publish(comm::ANDROID_EMULATOR,
			ClientRegistrationEventArgs(comm::ANDROID_EMULATOR,
					clientEntry));
	//start listening to the handler
	clientEntry->cnnHandler->start();
	Print() << "AndroidClient  Registered:" << std::endl;
	return true;
}

