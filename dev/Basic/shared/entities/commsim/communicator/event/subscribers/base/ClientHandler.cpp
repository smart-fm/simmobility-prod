//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ClientHandler.cpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#include "ClientHandler.hpp"
#include "entities/commsim/communicator/broker/Broker.hpp"
namespace sim_mob {

using namespace sim_mob::event;

ClientHandler::ClientHandler(sim_mob::Broker & broker_):broker(broker_) {
	// TODO Auto-generated constructor stub

}
sim_mob::Broker &ClientHandler::getBroker()
{
	return broker;
}

void ClientHandler::OnLocation(EventId id, Context context, EventPublisher* sender, const LocationEventArgs& argums){
//	 std::string locJson = argums.ToJSON();
	 Json::Value locJson = argums.ToJSON();
   //now send to broker's buffer
   getBroker().insertSendBuffer(cnnHandler, locJson);
}

void ClientHandler::OnTime(EventId id, EventPublisher* sender, const TimeEventArgs& args){
//   std::string timeJson = args.ToJSON();
   Json::Value timeJson = args.ToJSON();
   if(args.time.frame() > 158)
   {
	   int i = 0;
   }
   //now send to broker's buffer
   getBroker().insertSendBuffer(cnnHandler, timeJson);
}

ClientHandler::~ClientHandler() {

	cnnHandler.reset();
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
