//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include "util/DailyTime.hpp"

namespace sim_mob {

/**
 * Lightweight class to hold a bus schedule.
 */
class BusSchedule {
public:
	std::string tripid;
  	sim_mob::DailyTime startTime;

  	//We don't need this; we'll just save it while loading. ~Seth
  	//std::string TMP_startTimeStr;

  	//Don't both defining an empty constructor; we get a default one anyway. ~Seth
  	//BusSchedule() {}
  };
	
}
