/*
 * TimePublisher.cpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#include "TimePublisher.hpp"
#include "entities/commsim/communicator/broker/Common.hpp"

namespace sim_mob {

TimePublisher::TimePublisher() {
	RegisterEvent(COMMEID_TIME);
	myService = sim_mob::SIMMOB_SRV_TIME;
}
//void TimePublisher::publish(sim_mob::Broker &broker,sim_mob::registeredClient &subscriber, timeslice now){
//	std::string time = sim_mob::JsonParser::makeTimeData(now.frame());
//	sim_mob::DataElement data = sim_mob::makeDataElement((unsigned int)(subscriber.agent), subscriber.cnnHandler.get(), time);
//	broker.getSendBuffer().add(data);
//}
TimePublisher::~TimePublisher() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
