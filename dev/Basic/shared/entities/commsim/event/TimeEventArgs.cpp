//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TimeEventArgs.cpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#include "TimeEventArgs.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob;

sim_mob::TimeEventArgs::TimeEventArgs(timeslice time_): time(time_)
{
}

sim_mob::TimeEventArgs::~TimeEventArgs()
{
}

Json::Value sim_mob::TimeEventArgs::ToJSON() const
{
	return sim_mob::JsonParser::makeTimeData(time.frame(), ConfigManager::GetInstance().FullConfig().baseGranMS());
}
