//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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

class PT_bus_dispatch_freq {
public:
	std::string frequency_id;
	std::string route_id;
	sim_mob::DailyTime start_time;
	sim_mob::DailyTime end_time;
	int headway_sec;
};

class PT_bus_routes {
public:
	std::string route_id;
	std::string link_id;
	int link_sequence_no;
};

class PT_bus_stops {
public:
	std::string route_id;
	std::string busstop_no;
	int busstop_sequence_no;
};

}
