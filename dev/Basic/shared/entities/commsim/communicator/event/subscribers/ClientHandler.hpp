/*
 * ClientHandler.hpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#ifndef CLIENTHANDLER_HPP_
#define CLIENTHANDLER_HPP_

#include "event/EventListener.hpp"

namespace sim_mob {

class ClientHandler: public sim_mob::EventListener {
public:
	ClientHandler();
	virtual ~ClientHandler();
    void OnTime(EventId id, EventPublisher* sender, const TimeEventArgs& args);
};

} /* namespace sim_mob */
#endif /* CLIENTHANDLER_HPP_ */
