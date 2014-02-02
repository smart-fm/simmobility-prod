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
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "entities/commsim/broker/base/Common.hpp"
#include "entities/commsim/comm_support/AgentCommUtility.hpp"
#include "event/EventPublisher.hpp"
#include "entities/commsim/event/RegionsAndPathEventArgs.hpp"

using namespace sim_mob;





AndroidClientRegistration::AndroidClientRegistration(/*ConfigParams::ClientType type_*/) : ClientRegistrationHandler(/*ConfigParams::ANDROID_EMULATOR*/){
	// TODO Auto-generated constructor stub
}

bool AndroidClientRegistration::initialEvaluation(sim_mob::Broker& broker,AgentsList::type &registeredAgents){

	//some checks to avoid calling this method unnecessarily
	if (broker.getClientWaitingListSize()==0
			|| registeredAgents.empty()
			|| usedAgents.size() == registeredAgents.size()) {
		Print()
				<< "AndroidClientRegistration::handle initial failure, returning false"
				<< broker.getClientWaitingListSize() << "-"
				<< registeredAgents.size() << "-"
				<< usedAgents.size() << std::endl;
		return false;
	}
	return true;
}

bool AndroidClientRegistration::findAFreeAgent(AgentsList::type &registeredAgents,AgentsList::type::iterator &freeAgent){
	//find the first free agent(someone who is not yet been associated to an andriod client)
	freeAgent = registeredAgents.begin();
	AgentsList::type::iterator it_end = registeredAgents.end();
	for (; freeAgent != it_end; freeAgent++) {
		if (usedAgents.find(freeAgent->second.agent) == usedAgents.end()) {
			Print() << "Agent[" << freeAgent->second.agent->getId() << "]["<< freeAgent->second.agent << "] is free to be used" << std::endl;
					return true;
		}
	}

	Print()
					<< "AndroidClientRegistration::handle couldn't find a free agent among ["
					<< registeredAgents.size() << "], returning false" << std::endl;
	return false;
}

boost::shared_ptr<ClientHandler> AndroidClientRegistration::makeClientHandler(boost::shared_ptr<sim_mob::ConnectionHandler> connHandle, sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest &request, sim_mob::AgentInfo freeAgent)
{
	//Create a ClientHandler pointing to the Broker.
	boost::shared_ptr<ClientHandler> clientEntry(new ClientHandler(broker, connHandle, freeAgent.comm, freeAgent.agent, request.clientID));
	clientEntry->setRequiredServices(request.requiredServices);

	//Subscribe to relevant services for each required service.
	//TODO: This is *probably* correct, since we are attaching it to the clientEntry.
	sim_mob::Services::SIM_MOB_SERVICE srv;
	sim_mob::event::EventPublisher& publisher = broker.getPublisher();
	bool regionSupportRequired = false;
	BOOST_FOREACH(srv, request.requiredServices) {
		switch (srv) {
			case sim_mob::Services::SIMMOB_SRV_TIME:
				publisher.subscribe(COMMEID_TIME, clientEntry.get(), &ClientHandler::sendJsonToBroker, clientEntry->agent);
				break;
			case sim_mob::Services::SIMMOB_SRV_LOCATION:
				publisher.subscribe(COMMEID_LOCATION, clientEntry.get(), &ClientHandler::sendJsonToBroker, clientEntry->agent);
				break;
			case sim_mob::Services::SIMMOB_SRV_REGIONS_AND_PATH:
				publisher.subscribe(COMMEID_REGIONS_AND_PATH, clientEntry.get(), &ClientHandler::sendJsonToBroker, clientEntry->agent);
				break;
			default:
				Warn() <<"Android client requested service which could not be provided.\n"; break;
		}
	}

	//also, add the client entry to broker(for message handler purposes)
	broker.insertClientList(request.clientID, comm::ANDROID_EMULATOR, clientEntry);

	//add this agent to the list of the agents who are associated with a android emulator client
	usedAgents.insert(freeAgent.agent);

	//publish an event to inform- interested parties- of the registration of a new android client
	broker.getRegistrationPublisher().publish(comm::ANDROID_EMULATOR, ClientRegistrationEventArgs(comm::ANDROID_EMULATOR, clientEntry));
	return clientEntry;
}

bool AndroidClientRegistration::handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest &request, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn) {
	//This part is locked in fear of registered agents' iterator invalidation in the middle of the process
	AgentsList::Mutex registered_agents_mutex;
	AgentsList::type &registeredAgents = broker.getRegisteredAgents(&registered_agents_mutex);
	AgentsList::Lock lock(registered_agents_mutex);

	//some checks to avoid calling this method unnecessarily
	if(!initialEvaluation(broker,registeredAgents)) {
		return false;
	}

	AgentsList::type::iterator freeAgent;
	if (!findAFreeAgent(registeredAgents, freeAgent)) {
		return false;
	}

	//use it to create a client entry
	boost::shared_ptr<ClientHandler> clientEntry = makeClientHandler(existingConn, broker,request,freeAgent->second);

	//Inform the client we are ready to proceed.
	clientEntry->connHandle->forwardReadyMessage(*clientEntry);

	Print() << "AndroidClient  Registered. Multiplexed socket? " <<(existingConn?"Yes":"No") << std::endl;
	return true;
}

AndroidClientRegistration::~AndroidClientRegistration() {
	// TODO Auto-generated destructor stub
}

