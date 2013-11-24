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

namespace sim_mob {

class Broker;


class AndroidClientRegistration: public sim_mob::ClientRegistrationHandler {
	std::set<Agent*> usedAgents;
public:
	AndroidClientRegistration();
	virtual bool handle(sim_mob::Broker&, sim_mob::ClientRegistrationRequest);
	virtual ~AndroidClientRegistration();
};


} /* namespace sim_mob */
