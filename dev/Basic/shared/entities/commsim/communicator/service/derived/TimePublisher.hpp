//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TimePublisher.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/communicator/service/base/Publisher.hpp"
#include "event/EventPublisher.hpp"
namespace sim_mob {

class TimePublisher: public sim_mob::Publisher  {
public:
	TimePublisher();
	virtual ~TimePublisher();
};

} /* namespace sim_mob */
