/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>
#include "util/DailyTime.hpp"

namespace sim_mob {


class PT_trip {
public:
	std::string trip_id;
	std::string service_id;
	std::string route_id;
	sim_mob::DailyTime start_time;
	sim_mob::DailyTime end_time;

};

}
