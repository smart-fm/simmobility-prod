//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * SImulationScenario.hpp
 * Target:
 * currently not used
 */

#pragma once

#include <string>

namespace sim_mob
{

/**
 * \author Xu Yan
 */
class SimulationScenario
{
public:
	std::string day_of_week;
	std::string from_time;
	std::string to_time;
	std::string holiday;
	std::string road_network;
	std::string incident;
	std::string weather;
};
}
