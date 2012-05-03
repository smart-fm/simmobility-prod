/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "../all_includes.hpp"

namespace sim_mob_partitioning {
class Scenario
{
public:
	std::string road_network_name;
	std::string day_in_week;
	std::string from_time;
	std::string to_time;
	std::string is_holiday;
	std::string has_incident;
	std::string weather;
};
}
