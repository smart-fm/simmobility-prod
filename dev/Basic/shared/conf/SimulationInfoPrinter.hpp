#pragma once

#include <fstream>
#include <string>

#include "conf/ConfigManager.hpp"

namespace sim_mob
{

class SimulationInfoPrinter : private boost::noncopyable
{
public:
	SimulationInfoPrinter(ConfigParams& cfg, const std::string& filename);

	/**
	 * @brief printSimulationInfo - Prints the simulation information
	 */
	void printSimulationInfo();
private:
	/**
	 * @brief printSimulationTimeInfo - Prints the simulation time information
	 */
	void printSimulationTimeInfo();

	/** The output stream to which we are writing the file */
	mutable std::ofstream out;

	/** The configuration parameters */
	ConfigParams &cfg;
};

}
