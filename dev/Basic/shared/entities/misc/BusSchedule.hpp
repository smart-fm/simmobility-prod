/* Copyright Singapore-MIT Alliance for Research and Technology */

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
  	std::string TMP_startTimeStr;
  	BusSchedule() {}
  };
	
}
