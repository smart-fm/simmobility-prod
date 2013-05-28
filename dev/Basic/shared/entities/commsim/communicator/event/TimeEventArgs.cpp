/*
 * TimeEventArgs.cpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#include "TimeEventArgs.hpp"

namespace sim_mob {

TimeEventArgs::TimeEventArgs(timeslice time): time(time){
}

TimeEventArgs::~TimeEventArgs() {
}

std::string TimeEventArgs::ToJSON(){
	std::string time = sim_mob::JsonParser::makeTimeData(now.frame());
//	sim_mob::DataElement data = sim_mob::makeDataElement((unsigned int)(subscriber.agent), subscriber.cnnHandler.get(), time);
	//return time;
}
}
