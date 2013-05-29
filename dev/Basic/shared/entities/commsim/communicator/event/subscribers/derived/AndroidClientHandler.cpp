/*
 * AndroidClientHandler.cpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#include "AndroidClientHandler.hpp"
#include "entities/commsim/communicator/broker/Broker.hpp"

namespace sim_mob {

AndroidClientHandler::AndroidClientHandler(sim_mob::Broker & broker_):ClientHandler(broker_) {
	// TODO Auto-generated constructor stub

}

AndroidClientHandler::~AndroidClientHandler() {
	// TODO Auto-generated destructor stub
}

void AndroidClientHandler::OnTime(EventId id, EventPublisher* sender, const TimeEventArgs& args){
   std::string timeJson = args.ToJSON();
   sim_mob::DataElement data = sim_mob::makeDataElement(cnnHandler, timeJson);
   //now send to broker's buffer
   getBroker().insertSendBuffer(data);
}
} /* namespace sim_mob */
