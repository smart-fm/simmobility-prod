/*
 * ClientRegistrationHandler.hpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#ifndef CLINETHANDLER_HPP_
#define CLINETHANDLER_HPP_
#include "ClientRegistration.hpp"
#include "ClientRegistrationFactory.hpp"
namespace sim_mob {
class Broker;

class ClientRegistrationHandler {
public:
	ClientRegistrationHandler();
	virtual bool handle(sim_mob::Broker&, sim_mob::ClientRegistrationRequest) = 0;
	virtual ~ClientRegistrationHandler();
};

} /* namespace sim_mob */
#endif /* CLINETHANDLER_HPP_ */
