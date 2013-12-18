//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * NS3ClientRegistration.hpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/client/base/ClientRegistration.hpp"
#include "entities/commsim/Broker.hpp"

namespace sim_mob {
class ClientHandler;

class NS3ClientRegistration: public sim_mob::ClientRegistrationHandler  {
public:
	NS3ClientRegistration(/*ConfigParams::ClientType type_ = ConfigParams::NS3_SIMULATOR*/);
	virtual bool initialEvaluation(sim_mob::Broker& broker,AgentsList::type &registeredAgents);
	virtual boost::shared_ptr<ClientHandler> makeClientHandler(sim_mob::Broker&,sim_mob::ClientRegistrationRequest &,sim_mob::AgentInfo agent = sim_mob::AgentInfo());
	void sendAgentsInfo(sim_mob::Broker& broker, boost::shared_ptr<ClientHandler> clientEntry);
	virtual bool handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest &request);
	virtual ~NS3ClientRegistration();
};

} /* namespace sim_mob */
