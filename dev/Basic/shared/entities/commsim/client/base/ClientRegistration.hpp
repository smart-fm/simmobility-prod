//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include <set>
#include <map>
#include <string>

#include <boost/shared_ptr.hpp>
#include "entities/commsim/service/Services.hpp"
#include "event/args/EventArgs.hpp"

namespace sim_mob {

class Broker;
class ClientHandler;
class ConnectionHandler;


///Simple struct to hold a registration request for a client (Android/ns-3).
struct ClientRegistrationRequest {
	std::string clientID;
	std::string client_type; ///<ns-3 or android.
	std::set<sim_mob::Services::SIM_MOB_SERVICE> requiredServices;
};


///Simple EventArgs wrapper containing the ClientHandler for a given agent registration.
class ClientRegistrationEventArgs: public sim_mob::event::EventArgs {
public:
	ClientRegistrationEventArgs(boost::shared_ptr<ClientHandler>& client) : client(client) {}
	virtual ~ClientRegistrationEventArgs() {}

	boost::shared_ptr<ClientHandler> client;
};


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
	virtual ~ClientRegistrationHandler() {}
	virtual bool handle(sim_mob::Broker&, sim_mob::ClientRegistrationRequest&, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn) = 0;
};



}


