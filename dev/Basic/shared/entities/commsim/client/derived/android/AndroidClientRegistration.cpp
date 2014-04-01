//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "AndroidClientRegistration.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "entities/commsim/broker/Common.hpp"
#include "event/EventPublisher.hpp"
#include "entities/commsim/event/RegionsAndPathEventArgs.hpp"

using namespace sim_mob;



bool AndroidClientRegistration::initialEvaluation(sim_mob::Broker& broker,AgentsList::type &registeredAgents)
{
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

	Print() << "AndroidClientRegistration::handle couldn't find a free agent among ["
			<< registeredAgents.size() << "], returning false" << std::endl;
	return false;
}

boost::shared_ptr<ClientHandler> AndroidClientRegistration::makeClientHandler(boost::shared_ptr<sim_mob::ConnectionHandler> connHandle, sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest& request, const Agent* freeAgent)
{
	//Create a ClientHandler pointing to the Broker.
	boost::shared_ptr<ClientHandler> clientEntry(new ClientHandler(broker, connHandle,  freeAgent, request.clientID));
	clientEntry->setRequiredServices(request.requiredServices);

	//Subscribe to relevant services for each required service.
	//TODO: This is *probably* correct, since we are attaching it to the clientEntry.
	//sim_mob::Services::SIM_MOB_SERVICE srv;
	sim_mob::event::EventPublisher& publisher = broker.getPublisher();
	bool regionSupportRequired = false;

	for (std::set<sim_mob::Services::SIM_MOB_SERVICE>::const_iterator it=request.requiredServices.begin(); it!=request.requiredServices.end(); it++) {
	//BOOST_FOREACH(srv, request.requiredServices) {
		switch (*it) {
			case sim_mob::Services::SIMMOB_SRV_TIME:
				publisher.subscribe(COMMEID_TIME, clientEntry.get(), &ClientHandler::sendSerializedMessageToBroker, clientEntry->agent);
				break;
			case sim_mob::Services::SIMMOB_SRV_LOCATION:
				publisher.subscribe(COMMEID_LOCATION, clientEntry.get(), &ClientHandler::sendSerializedMessageToBroker, clientEntry->agent);
				break;
			case sim_mob::Services::SIMMOB_SRV_REGIONS_AND_PATH:
				publisher.subscribe(COMMEID_REGIONS_AND_PATH, clientEntry.get(), &ClientHandler::sendSerializedMessageToBroker, clientEntry->agent);
				break;
			default:
				Warn() <<"Android client requested service which could not be provided.\n"; break;
		}
	}

	//also, add the client entry to broker(for message handler purposes)
	broker.insertClientList(request.clientID, comm::ANDROID_EMULATOR, clientEntry);

	//add this agent to the list of the agents who are associated with a android emulator client
	usedAgents.insert(freeAgent);

	return clientEntry;
}

bool AndroidClientRegistration::handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest& request, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn) {
	//This part is locked in fear of registered agents' iterator invalidation in the middle of the process
	AgentsList::Mutex registered_agents_mutex;
	AgentsList::type& registeredAgents = broker.getRegisteredAgents(&registered_agents_mutex);
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
	boost::shared_ptr<ClientHandler> clientEntry = makeClientHandler(existingConn, broker,request,freeAgent->second.agent);

	//Inform the client we are ready to proceed.
	clientEntry->connHandle->forwardReadyMessage(*clientEntry);

	Print() << "AndroidClient  Registered. Multiplexed socket? " <<(existingConn?"Yes":"No") << std::endl;
	return true;
}
