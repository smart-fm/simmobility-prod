//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * LocationPublisher.cpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#include "LocationPublisher.hpp"
#include "entities/commsim/communicator/broker/Common.hpp"

namespace sim_mob {

LocationPublisher::LocationPublisher() {
//	myService = sim_mob::SIMMOB_SRV_LOCATION;
	RegisterEvent(COMMEID_LOCATION);
}

LocationPublisher::~LocationPublisher() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
