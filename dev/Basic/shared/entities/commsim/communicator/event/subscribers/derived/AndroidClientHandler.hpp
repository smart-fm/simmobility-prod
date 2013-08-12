/*
 * AndroidClientHandler.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/communicator/event/subscribers/base/ClientHandler.hpp"

namespace sim_mob {

class AndroidClientHandler: public ClientHandler {
public:
	AndroidClientHandler(sim_mob::Broker &);
	virtual ~AndroidClientHandler();

    void OnTime(event::EventId id, event::EventPublisher* sender, const TimeEventArgs& args);
};

} /* namespace sim_mob */
