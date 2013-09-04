/*
 * TimeEventArgs.cpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#include "TimeEventArgs.hpp"
//#include "entities/commsim/serialization/Serialization.hpp"

namespace sim_mob {

TimeEventArgs::TimeEventArgs(timeslice time_): time(time_){
}

TimeEventArgs::~TimeEventArgs() {
}

Json::Value TimeEventArgs::ToJSON() const{
	Json::Value mytime = sim_mob::JsonParser::makeTimeData(time.frame(), ConfigParams::GetInstance().baseGranMS());
	return mytime;
}
}
