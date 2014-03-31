//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TimeEventArgs.hpp"

#include "entities/commsim/serialization/CommsimSerializer.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob;

sim_mob::TimeEventArgs::TimeEventArgs(timeslice time): time(time)
{
}

sim_mob::TimeEventArgs::~TimeEventArgs()
{
}

std::string sim_mob::TimeEventArgs::serialize() const
{
	return sim_mob::CommsimSerializer::makeTimeData(time.frame(), ConfigManager::GetInstance().FullConfig().baseGranMS());
}
