//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ClientRegistration.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#pragma once

#include <set>
#include <map>
#include <string>

#include <boost/shared_ptr.hpp>
#include "entities/Agent.hpp"
#include "entities/commsim/service/Services.hpp"
#include "event/EventPublisher.hpp"

namespace sim_mob {

class ConnectionHandler;

/**
 * ClientRegistrationRequest class. No documentation provided.
 */
class ClientRegistrationRequest {
public:
	std::string clientID;
	std::string client_type; //ns3, android emulator, FMOD etc
	std::set<sim_mob::Services::SIM_MOB_SERVICE> requiredServices;
};





class Broker;

/**
 *      This Class is abstract. Its derived classed are responsible to process the registration request.
 *      Such a request has been previously issued following a client connecting to simmobility.
 *      registration, in this context, means adding a client to the list of valid clients
 *      in the Broker. Processing a registration request, generally, includes an initial
 *      evaluation, associating the client to a simmobility agent, creating a proper
 *      client handler and finally do some post processing like informing the client of
 *      the success of its request.
 *      the main method is handle(). the rest of the methods are usually helpers.
 *
 *      Note that if "existingConn" is non-null, the given "existing" ConnectionHandler is used
 *        to multiplex reads and writes from the new ClientHandler.
 */
class ClientRegistrationHandler {
public:
	ClientRegistrationHandler();
	virtual ~ClientRegistrationHandler();
	virtual bool handle(sim_mob::Broker&, sim_mob::ClientRegistrationRequest&, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn) = 0;
	virtual void postProcess(sim_mob::Broker& broker);
	static sim_mob::event::EventPublisher & getPublisher();

private:
	comm::ClientType type;
};



class ClientHandler;

/**
 * ClientRegistrationEventArgs class. No documentation provided.
 */
class ClientRegistrationEventArgs: public sim_mob::event::EventArgs {
	boost::shared_ptr<ClientHandler> client;
	comm::ClientType type;
public:
	ClientRegistrationEventArgs(comm::ClientType, boost::shared_ptr<ClientHandler>&);
	boost::shared_ptr<ClientHandler> getClient() const;
	comm::ClientType getClientType() const;
	virtual ~ClientRegistrationEventArgs();
};

}//namespace sim_mob

