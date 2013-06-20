/*
 * LocationPublisher.cpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#include "LocationPublisher.hpp"
#include "entities/commsim/communicator/broker/Common.hpp"

namespace sim_mob {

LocationPublisher::LocationPublisher() {
//	myService = sim_mob::SIMMOB_SRV_LOCATION;
	RegisterEvent(COMMEID_LOCATION);
}

//void LocationPublisher::publish(sim_mob::Broker & broker,sim_mob::registeredClient & subscriber, timeslice now)
//{
//	int x,y;
//	x = subscriber.agent->xPos.get();
//	y = subscriber.agent->yPos.get();
//	std::string location_ = sim_mob::JsonParser::makeLocationData(x,y);
//	sim_mob::DataElement data = sim_mob::makeDataElement((unsigned int)(subscriber.agent), subscriber.cnnHandler.get(), location_);
//	broker.getSendBuffer().add(data);
//}
LocationPublisher::~LocationPublisher() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
