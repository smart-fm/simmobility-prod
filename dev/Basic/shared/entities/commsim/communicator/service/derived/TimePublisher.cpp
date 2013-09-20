//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TimePublisher.cpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#include "TimePublisher.hpp"
#include "entities/commsim/communicator/broker/Common.hpp"

namespace sim_mob {

TimePublisher::TimePublisher() {
	RegisterEvent(COMMEID_TIME);
//	myService = sim_mob::SIMMOB_SRV_TIME;
}
TimePublisher::~TimePublisher() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
