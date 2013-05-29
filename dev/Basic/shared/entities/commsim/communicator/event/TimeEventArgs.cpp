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
	return mytime;
}
}
