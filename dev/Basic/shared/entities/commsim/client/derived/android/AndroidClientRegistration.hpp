//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/commsim/client/base/ClientRegistration.hpp"
#include "entities/commsim/broker/Broker.hpp"
namespace sim_mob {


/**
 * This Class is responsible to process the registration request. Such a request
 * has been previously issued following an android client connecting to simmobility.
 * registration, in this context, means adding a client to the list of valid clients
 * in the Broker. Processing a registration request, generally, includes an initial
 * evaluation, associating the client to a simmobility agent, creating a proper
 * client handler and finally do some post processing like informing the client of
 * the success of its request.
 * the main method is handle(). the rest of the methods are usually helpers.
 */
class AndroidClientRegistration: public sim_mob::ClientRegistrationHandler {
public:
	AndroidClientRegistration();
	/**
	 *  helper function used in handle() method to do some checks
	 *  in order to avoid calling this method unnecessarily
	 */
	bool initialEvaluation(sim_mob::Broker& broker,AgentsList::type &registeredAgents);
	/**
	 * actual handler used to register a client of android type: ANDROID_EMULATOR
	 */
	virtual bool handle(sim_mob::Broker&, sim_mob::ClientRegistrationRequest&, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn);
	/**
	 * helper function used in handle() method to find a simmobility agent which has not been associated to this type of a client
	 */
	virtual bool findAFreeAgent(AgentsList::type &registeredAgents,AgentsList::type::iterator &freeAgent);

	/**
	 * helper function used in handle() method to prepare and return a sim_mob::ClientHandler
	 * If connHandle is null, create a new connection handler. Otherwise, just re-use it.
	 */
	virtual boost::shared_ptr<ClientHandler> makeClientHandler(boost::shared_ptr<sim_mob::ConnectionHandler> connHandle, sim_mob::Broker&,sim_mob::ClientRegistrationRequest &, const sim_mob::Agent* freeAgent);

	virtual ~AndroidClientRegistration();

private:
	std::set<const sim_mob::Agent*> usedAgents;
};


} /* namespace sim_mob */
