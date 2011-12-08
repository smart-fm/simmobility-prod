/*
 * SImulationScenario.hpp
 * Target:
 * currently not used
 */

#pragma once

#include <string>

namespace sim_mob
{
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
