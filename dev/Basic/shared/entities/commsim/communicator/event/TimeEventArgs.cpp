/*
 * TimeEventArgs.cpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#include "TimeEventArgs.hpp"
#include "entities/commsim/communicator/serialization/Serialization.hpp"

namespace sim_mob {

TimeEventArgs::TimeEventArgs(timeslice time_): time(time_){
}

TimeEventArgs::~TimeEventArgs() {
}

std::string TimeEventArgs::ToJSON() const{
	std::string mytime = sim_mob::JsonParser::makeTimeData(time.frame());
	sim_mob::DataElement data = sim_mob::makeDataElement((unsigned int)(subscriber.agent), subscriber.cnnHandler.get(), mytime);
	return mytime;
}
}
