//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * NS3ClientRegistration.hpp
 *      This Class is responsible to process the registration request.
 *      Such a request has been previously issued following a client connecting to simmobility.
 *      registration, in this context, means adding a NS3 client to the list of valid clients
 *      in the Broker. Processing a registration request, generally, includes an initial
 *      evaluation, associating the client to a simmobility agent, creating a proper
 *      client handler and finally do some post processing like informing the client of
 *      the success of its request.
 *      the main method is handle(). the rest of the methods are usually helpers.
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/client/base/ClientRegistration.hpp"
#include "entities/commsim/broker/Broker.hpp"

namespace sim_mob {
class ClientHandler;

class NS3ClientRegistration: public sim_mob::ClientRegistrationHandler  {
	//temporary place holder for inter method data passing
	boost::shared_ptr<ClientHandler> clientHandler;

public:
	NS3ClientRegistration(/*ConfigParams::ClientType type_ = ConfigParams::NS3_SIMULATOR*/);

	/**
	 *  helper function used in handle() method to do some checks
	 *  in order to avoid calling this method unnecessarily
	 */
	virtual bool initialEvaluation(sim_mob::Broker& broker,AgentsList::type &registeredAgents);
	/**
	 * helper function used in handle() method to prepare and return a sim_mob::ClientHandler
	 */
	virtual boost::shared_ptr<ClientHandler> makeClientHandler(boost::shared_ptr<sim_mob::ConnectionHandler> existingConn, sim_mob::Broker&,sim_mob::ClientRegistrationRequest &);
	/**
	 * Helper function used to send simmobility agents information to ns3 in json format
	 */
	void sendAgentsInfo(sim_mob::Broker& broker, boost::shared_ptr<ClientHandler> clientEntry);

	/**
	 * actual handler used to register a client of ns3 type: NS3_SIMULATOR
	 */
	virtual bool handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest &request, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn);
	virtual ~NS3ClientRegistration();
};

} /* namespace sim_mob */
