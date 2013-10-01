//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * NS3ClientRegistration.hpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#ifndef NS3ClientRegistration_HPP_
#define NS3ClientRegistration_HPP_
#include "entities/commsim/client/base/ClientRegistration.hpp"
#include "entities/commsim/broker/Broker.hpp"

namespace sim_mob {

class NS3ClientRegistration: public sim_mob::ClientRegistrationHandler  {
public:
	NS3ClientRegistration(/*ConfigParams::ClientType type_ = ConfigParams::NS3_SIMULATOR*/);
	virtual bool handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest request);
	virtual ~NS3ClientRegistration();
};

} /* namespace sim_mob */
#endif /* NS3ClientRegistration_HPP_ */
