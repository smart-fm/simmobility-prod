/*
 * DatabaseConfiguration.hpp
 *
 *  Created on: 10-Apr-2012
 *      Author: xuyan
 */

#pragma once

#include "../all_includes.hpp"

namespace sim_mob_partitioning {
class DatabaseConfiguration
{
public:
	std::string node_network_name;
	std::string day_in_week;
	std::string from_time;
	std::string to_time;
	std::string holiday;
	std::string incident;
	std::string weather;
};
}
