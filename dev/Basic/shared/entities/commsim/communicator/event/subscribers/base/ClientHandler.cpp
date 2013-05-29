/*
 * ClientHandler.cpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#include "ClientHandler.hpp"

namespace sim_mob {

ClientHandler::ClientHandler(sim_mob::Broker & broker_):broker(broker_) {
	// TODO Auto-generated constructor stub

}
sim_mob::Broker &ClientHandler::getBroker()
{
	return broker;
}
ClientHandler::~ClientHandler() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
