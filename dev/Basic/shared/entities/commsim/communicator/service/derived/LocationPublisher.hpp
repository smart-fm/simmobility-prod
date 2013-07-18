/*
 * LocationPublisher.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/communicator/service/base/Publisher.hpp"

namespace sim_mob {

class LocationPublisher: public sim_mob::Publisher {
public:
	LocationPublisher();
//	void publish(sim_mob::Broker &,sim_mob::registeredClient &, timeslice);
	virtual ~LocationPublisher();
};

} /* namespace sim_mob */
