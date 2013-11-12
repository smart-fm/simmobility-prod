//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AndroidClientHandler.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"

namespace sim_mob {

class AndroidClientHandler: public ClientHandler {
public:
	AndroidClientHandler(sim_mob::Broker &);
	virtual ~AndroidClientHandler();

    void OnTime(sim_mob::event::EventId id, sim_mob::event::EventPublisher* sender, const TimeEventArgs& args);
};

} /* namespace sim_mob */
