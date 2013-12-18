//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AndroidClientRegistration.hpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/client/base/ClientRegistration.hpp"
#include "entities/commsim/Broker.hpp"
namespace sim_mob {



class AndroidClientRegistration: public sim_mob::ClientRegistrationHandler {
	std::set<sim_mob::Agent*> usedAgents;
public:
	AndroidClientRegistration();
	bool initialEvaluation(sim_mob::Broker& broker,AgentsList::type &registeredAgents);
	virtual bool handle(sim_mob::Broker&, sim_mob::ClientRegistrationRequest&);
	virtual bool findAFreeAgent(AgentsList::type &registeredAgents,AgentsList::type::iterator &freeAgent);
	virtual boost::shared_ptr<ClientHandler> makeClientHandler(sim_mob::Broker&,sim_mob::ClientRegistrationRequest &,sim_mob::AgentInfo freeAgent);

	virtual ~AndroidClientRegistration();
};


} /* namespace sim_mob */
