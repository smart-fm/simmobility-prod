/*
 * ClientHandler.cpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#include "ClientHandler.hpp"

namespace sim_mob {

ClientHandler::ClientHandler() {
	// TODO Auto-generated constructor stub

}

ClientHandler::~ClientHandler() {
	// TODO Auto-generated destructor stub
}

void OnTime(EventId id,  EventPublisher* sender, const TimeEventArgs& args){
   std::string timeJson = args.ToJSON();
   conn.send(timeJson);

}

} /* namespace sim_mob */
