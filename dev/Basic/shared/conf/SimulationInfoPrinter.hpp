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
	 * Prints the simulation information
	 */
	void printSimulationInfo();
private:
	/** The output stream to which we are writing the file */
	std::ofstream out;

	/** The configuration parameters */
	ConfigParams &cfg;
};

}
