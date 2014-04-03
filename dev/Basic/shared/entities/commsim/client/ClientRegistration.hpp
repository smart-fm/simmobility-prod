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

class Agent;
struct AgentInfo;
class BrokerBase;
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
 */
class ClientRegistrationHandler {
public:
	bool handle(BrokerBase&, sim_mob::ClientRegistrationRequest&, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn, bool isNs3Client);

private:
	///Helper function: Find an agent from the list of registeredAgents that is not in our set of usedAgents.
	///Returns null if no such agent exists.
	const Agent* findAFreeAgent(const std::map<const Agent*, AgentInfo>& registeredAgents);

	/**
	 * helper function used in handle() method to prepare and return a sim_mob::ClientHandler
	 * If connHandle is null, create a new connection handler. Otherwise, just re-use it.
	 */
	boost::shared_ptr<ClientHandler> makeClientHandler(boost::shared_ptr<sim_mob::ConnectionHandler> connHandle, BrokerBase&, sim_mob::ClientRegistrationRequest &, const sim_mob::Agent* freeAgent, bool isNs3Client);

	/**
	 * Helper function used to send simmobility agents information to ns3 in json format
	 */
	void sendAgentsInfo(const std::map<const Agent*, AgentInfo>& agents, boost::shared_ptr<ClientHandler> clientEntry);


private:
	std::set<const Agent*> usedAgents;

};



}


