/*
 * ClientHandler.cpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#include "ClientHandler.hpp"
#include "entities/commsim/communicator/broker/Broker.hpp"
namespace sim_mob {

ClientHandler::ClientHandler(sim_mob::Broker & broker_):broker(broker_) {
	// TODO Auto-generated constructor stub

}
sim_mob::Broker &ClientHandler::getBroker()
{
	return broker;
}

void ClientHandler::OnLocation(EventId id, EventPublisher* sender, const LocationEventArgs& args){
	 std::string locJson = args.ToJSON();
   sim_mob::DataElement data = sim_mob::makeDataElement(cnnHandler, locJson);
   //now send to broker's buffer
   getBroker().insertSendBuffer(data);
}

void ClientHandler::OnTime(EventId id, EventPublisher* sender, const TimeEventArgs& args){
   std::string timeJson = args.ToJSON();
   sim_mob::DataElement data = sim_mob::makeDataElement(cnnHandler, timeJson);
   //now send to broker's buffer
   getBroker().insertSendBuffer(data);
}

ClientHandler::~ClientHandler() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
