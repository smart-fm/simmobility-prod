//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Publisher.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/communicator/service/services.hpp"
#include <boost/assign/list_of.hpp>
#include "entities/commsim/communicator/broker/Broker.hpp"
#include "metrics/Frame.hpp"
#include "event/EventPublisher.hpp"
namespace sim_mob {
class Publisher : public sim_mob::event::EventPublisher {
private:
//	sim_mob::SIM_MOB_SERVICE myService;
public:
	Publisher();
//	virtual void publish(sim_mob::Broker&, sim_mob::registeredClient&, timeslice) = 0;
	virtual ~Publisher();
};

} /* namespace sim_mob */
