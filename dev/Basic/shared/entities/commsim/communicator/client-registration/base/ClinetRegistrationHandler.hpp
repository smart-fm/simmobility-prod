/*
 * ClientRegistrationHandler.hpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#ifndef CLINETHANDLER_HPP_
#define CLINETHANDLER_HPP_
#include "entities/commsim/communicator/broker/Broker.hpp"
#include <boost/foreach.hpp>

namespace sim_mob {

class ClientRegistrationHandler {
	const ClientType myType;
public:
	ClientRegistrationHandler(ClientType);
	virtual bool handle(sim_mob::Broker&, sim_mob::ClientRegistrationRequest) = 0;
	virtual ~ClientRegistrationHandler();
};

} /* namespace sim_mob */
#endif /* CLINETHANDLER_HPP_ */
