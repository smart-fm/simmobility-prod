/*
 * NS3ClientRegistration.hpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#ifndef NS3ClientRegistration_HPP_
#define NS3ClientRegistration_HPP_
#include "entities/commsim/communicator/client-registration/base/ClinetRegistrationHandler.hpp"
namespace sim_mob {

class NS3ClientRegistration: public sim_mob::ClientRegistrationHandler  {
public:
	NS3ClientRegistration(/*ClientType type_ = NS3_SIMULATOR*/);
	bool handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest request);
	virtual ~NS3ClientRegistration();
};

} /* namespace sim_mob */
#endif /* NS3ClientRegistration_HPP_ */
