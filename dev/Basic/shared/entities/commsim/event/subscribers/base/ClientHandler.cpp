/*
 * ClientHandler.cpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#include "ClientHandler.hpp"
#include "entities/commsim/broker/Broker.hpp"
namespace sim_mob {

ClientHandler::ClientHandler(sim_mob::Broker & broker_):broker(broker_) {
	valid = true;
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

void ClientHandler::OnAllLocations(EventId id, Context context, EventPublisher* sender, const AllLocationsEventArgs& argums){
//	 std::string locJson = argums.ToJSON();
	 Json::Value locJson = argums.ToJSON();
   //now send to broker's buffer
   getBroker().insertSendBuffer(cnnHandler, locJson);
}

void ClientHandler::OnTime(EventId id, EventPublisher* sender, const TimeEventArgs& args){
//   std::string timeJson = args.ToJSON();
   Json::Value timeJson = args.ToJSON();
   //now send to broker's buffer
   getBroker().insertSendBuffer(cnnHandler, timeJson);
}

bool ClientHandler::isValid() {
	return valid;
}

void ClientHandler::setValidation(bool value) {
	valid = value;
}

ClientHandler::~ClientHandler() {

//	cnnHandler.reset();
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
