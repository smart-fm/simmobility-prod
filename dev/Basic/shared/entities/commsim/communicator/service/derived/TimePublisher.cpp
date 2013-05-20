/*
 * TimePublisher.cpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#include "TimePublisher.hpp"

namespace sim_mob {

TimePublisher::TimePublisher() {
	// TODO Auto-generated constructor stub

}
void TimePublisher::publish(sim_mob::Broker *broker,sim_mob::subscription &subscriber, timeslice now){
	std::string time = sim_mob::JsonParser::makeTimeData(now.frame());
	sim_mob::DataElement data = sim_mob::makeDataElement((unsigned int)(subscriber.agent), subscriber.handler.get(), time);
	broker->getSendBuffer().add(data);
}
TimePublisher::~TimePublisher() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
