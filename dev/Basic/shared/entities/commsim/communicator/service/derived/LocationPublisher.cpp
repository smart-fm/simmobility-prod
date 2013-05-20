/*
 * LocationPublisher.cpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#include "LocationPublisher.hpp"

namespace sim_mob {

LocationPublisher::LocationPublisher() {
	// TODO Auto-generated constructor stub

}

void publish(sim_mob::Broker * broker,sim_mob::subscription & subscriber, timeslice now)
{
	int x,y;
	x = subscriber.agent->xPos.get();
	y = subscriber.agent->yPos.get();
	std::string location_ = sim_mob::JsonParser::makeLocationData(x,y);
	sim_mob::DataElement data = sim_mob::makeDataElement((unsigned int)(subscriber.agent), subscriber.handler.get(), location_);
	broker->getSendBuffer().add(data);
}
LocationPublisher::~LocationPublisher() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
