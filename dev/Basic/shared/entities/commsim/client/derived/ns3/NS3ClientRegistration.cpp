//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * NS3ClientRegistration.cpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#include "NS3ClientRegistration.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "event/EventPublisher.hpp"
#include "entities/commsim/broker/base/Common.hpp"
#include "entities/commsim/event/AllLocationsEventArgs.hpp"
#include "AgentsInfo.hpp"
#include <boost/foreach.hpp>

sim_mob::NS3ClientRegistration::NS3ClientRegistration() :
		ClientRegistrationHandler() {
}
sim_mob::NS3ClientRegistration::~NS3ClientRegistration() {
}

bool sim_mob::NS3ClientRegistration::initialEvaluation(sim_mob::Broker& broker,
		AgentsList::type &registeredAgents) {
	bool res = false;
	//add your conditions here
	res = broker.getClientWaitingListSize()>0;

	return res;
}

boost::shared_ptr<sim_mob::ClientHandler> sim_mob::NS3ClientRegistration::makeClientHandler(boost::shared_ptr<sim_mob::ConnectionHandler> existingConn,
		sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest &request, sim_mob::AgentInfo agent)
{
	boost::shared_ptr<ClientHandler> clientEntry(new ClientHandler(broker, existingConn, nullptr, nullptr, request.clientID));
	clientEntry->setRequiredServices(request.requiredServices);

	sim_mob::event::EventPublisher & p = broker.getPublisher();
	sim_mob::Services::SIM_MOB_SERVICE srv;
	int size_i = request.requiredServices.size();
	BOOST_FOREACH(srv, request.requiredServices) {
		switch (srv) {
		case sim_mob::Services::SIMMOB_SRV_TIME: {
//			 PublisherList::Value p = broker.getPublisher(sim_mob::Services::SIMMOB_SRV_TIME);
			p.subscribe(COMMEID_TIME, clientEntry.get(),
					&ClientHandler::sendJsonToBroker);
			break;
		}
		case sim_mob::Services::SIMMOB_SRV_ALL_LOCATIONS: {
//			PublisherList::Value p = broker.getPublisher(sim_mob::Services::SIMMOB_SRV_ALL_LOCATIONS);

			//NOTE: It does not seem like we even use the "Context" pointer, so I am switching
			//      this to a regular CALLBACK_HANDLER. Please review. ~Seth
			//p->subscribe(COMMEID_LOCATION,
			//	clientEntry.get(),
			//	&ClientHandler::OnEvent,
			//	COMMCID_ALL_LOCATIONS);
			p.subscribe(COMMEID_ALL_LOCATIONS, clientEntry.get(),
					&ClientHandler::sendJsonToBroker,
					(void*) COMMCID_ALL_LOCATIONS);
			break;
		}
		}
	}

	//also, add the client entry to broker(for message handler purposes)
	broker.insertClientList(clientEntry->clientId, comm::NS3_SIMULATOR, clientEntry);

	return clientEntry;

}

void sim_mob::NS3ClientRegistration::sendAgentsInfo(sim_mob::Broker& broker,
		boost::shared_ptr<ClientHandler> clientEntry) {
	//send some initial configuration information to NS3
	std::set<sim_mob::Entity *> keys;
	{			//multi-threaded section, need locking
		AgentsList::Mutex mutex;
		AgentsList::type & agents = broker.getRegisteredAgents(&mutex);
		AgentsList::Lock lock(mutex);
		//please mind the AgentInfo vs AgentsInfo
		//AgentInfo agent;
		for (AgentsList::type::iterator it = agents.begin(); it != agents.end();
				it++) {
			//BOOST_FOREACH(agent, agents) {
			keys.insert(it->second.agent);
		}
	}
//no lock and const_cast at the cost of a lot of copying
	AgentsInfo info;
	info.insertInfo(AgentsInfo::ADD_AGENT, keys);
	clientEntry->connHandle->sendImmediately(info.toJson());	//send synchronously
}

bool sim_mob::NS3ClientRegistration::handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest &request, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn)
{
	//For now, this only works on its own dedicated socket.
	//if (existingConn) {
	//	throw std::runtime_error("NS3ClientRegistration requires a unique socket connection.");
	//}

//	//This part is locked in fear of registered agents' iterator invalidation in the middle of the process
	AgentsList::Mutex *registered_agents_mutex;
	AgentsList::type &registeredAgents = broker.getRegisteredAgents(registered_agents_mutex);
	if (!initialEvaluation(broker, registeredAgents)) {
		return false;
	}

	//use it to create a client entry
	clientHandler = makeClientHandler(existingConn, broker, request);

	//AgentsInfo is required before READY
	sendAgentsInfo(broker, clientHandler);

	//Inform the handler we are ready to proceed.
	clientHandler->connHandle->forwardReadyMessage(*clientHandler);

	return true;
}

