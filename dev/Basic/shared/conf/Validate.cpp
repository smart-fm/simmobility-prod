/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

#include "Validate.hpp"

#include <sstream>

#include "conf/Config.hpp"

using std::map;
using std::set;
using std::vector;
using std::string;

using namespace sim_mob;


namespace {
//Helper: check Granularity consistency
void CheckGranularity(const string& title, const int childGran, const int parentGran) {
	if (childGran < parentGran) {
		std::stringstream msg;
		msg <<title <<" granularity (" <<childGran <<") is smaller than base granularity (" <<parentGran <<").";
		throw std::runtime_error(msg.str().c_str());
	}
	if (childGran % parentGran != 0) {
		std::stringstream msg;
		msg <<title <<" granularity (" <<childGran <<") is not a multiple of the base granularity (" <<parentGran <<").";
		throw std::runtime_error(msg.str().c_str());	}
}
} //End un-named namespace


sim_mob::Validate::Validate(Config& cfg) : cfg(cfg)
{
	//Ensure that all our granularities add up right.
	CheckAndSetGranularities();
}



void sim_mob::Validate::CheckAndSetGranularities() const
{
	Simulation& sim = cfg.simulation();
	const int BaseGran = sim.baseGranularity.ms();

	//Check each granularity.
	CheckGranularity("Agent", sim.agentGranularity.ms(), BaseGran);
	CheckGranularity("Signal", sim.signalGranularity.ms(), BaseGran);
	CheckGranularity("Total Runtime", sim.totalRuntime.ms(), BaseGran);
	CheckGranularity("Total Warmup", sim.totalWarmup.ms(), BaseGran);

	//Reset each granularity with baseGran.
	sim.agentGranularity.setBaseGranMS(BaseGran);
	sim.signalGranularity.setBaseGranMS(BaseGran);
	sim.totalRuntime.setBaseGranMS(BaseGran);
	sim.totalWarmup.setBaseGranMS(BaseGran);
}










