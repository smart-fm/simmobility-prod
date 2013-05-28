/*
 * AndroidClientRegistration.hpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#ifndef AndroidClientRegistration_HPP_
#define AndroidClientRegistration_HPP_

#include "entities/commsim/communicator/client-registration//base/ClinetRegistrationHandler.hpp"

namespace sim_mob {

class AndroidClientRegistration: public sim_mob::ClientRegistrationHandler {
	AgentsMap usedAgents;
public:
	AndroidClientRegistration(ClientType type_ = ANDROID_EMULATOR);
	bool handle(sim_mob::Broker&, sim_mob::ClientRegistrationRequest);
	virtual ~AndroidClientRegistration();
};

} /* namespace sim_mob */
#endif /* AndroidClientRegistration_HPP_ */
