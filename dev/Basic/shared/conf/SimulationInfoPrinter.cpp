#include "SimulationInfoPrinter.hpp"

using namespace sim_mob;

SimulationInfoPrinter::SimulationInfoPrinter(ConfigParams& cfg, const std::string& filename) : cfg(cfg), out(filename.c_str())
{

}

void SimulationInfoPrinter::printSimulationInfo()
{
	/// {"SimulationInfo":{"StartTime":"06:00:00","Granularity":"0.1"}}
	out << "\n{\"SimulationInfo\":{\"StartTime\":\"" << cfg.simStartTime().getStrRepr()
		<< "\",\"Granularity\":\"" << cfg.baseGranSecond() << "\"}}\n\n";
}
