/*
 * AndroidClientHandler.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#ifndef ANDROIDCLIENTHANDLER_HPP_
#define ANDROIDCLIENTHANDLER_HPP_

#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"

namespace sim_mob {

class AndroidClientHandler: public ClientHandler {
public:
	AndroidClientHandler(sim_mob::Broker &);
	virtual ~AndroidClientHandler();

    void OnTime(EventId id, EventPublisher* sender, const TimeEventArgs& args);
};

} /* namespace sim_mob */
#endif /* ANDROIDCLIENTHANDLER_HPP_ */
