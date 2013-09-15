//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
