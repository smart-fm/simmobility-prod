/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

#include "Validate.hpp"

#include <sstream>
#include <boost/lexical_cast.hpp>

#include "conf/Config.hpp"
#include "partitions/PartitionManager.hpp"
#include "entities/Agent.hpp"

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

	//Convert our properties from key/value strings to the appropriate object properties.
	CheckAndSetGeneralProps();
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



void sim_mob::Validate::CheckAndSetGeneralProps() const
{
	//As we retrieve properties, erase them.
	map<string, string>& props = cfg.system().genericProps;

	//Our "partitioning solution id" for MPI
	map<string, string>::iterator it = props.find("partitioning_solution_id");
	if (it!=props.end()) {
		//Needs to be an integer.
		int val = boost::lexical_cast<int>(it->second);

		//Only set if Partitioning is on (checking CMake properties is always valid)
		if (cfg.MPI_Enabled()) {
			PartitionManager::instance().partition_config->partition_solution_id = val;
		}

		//Erase from our map
		props.erase(it);
	}

	//Agent auto-id start number
	it = props.find("auto_id_start");
	if (it!=props.end()) {
		//Needs to be an integer.
		int val = boost::lexical_cast<int>(it->second);

		//Only set if >0
		if (val>0) {
			Agent::SetIncrementIDStartValue(val, true);
			cfg.system().startingAgentAutoID = val;
		}

		//Erase from our map
		props.erase(it);
	}


	//Our mutex enforcement strategy: buffered or locked.
	it = props.find("mutex_enforcement_strategy");
	if (it!=props.end()) {
		//Can only be "buffered" or "locked" for now.
		if (it->second=="buffered") {
			cfg.mutexStrategy() = MtxStrat_Buffered;
		} else if (it->second=="locked") {
			cfg.mutexStrategy() = MtxStrat_Locked;
		} else {
			std::stringstream msg;
			msg <<"Unknown mutex enforcement strategy: \"" <<it->second <<"\"";
			throw std::runtime_error(msg.str().c_str());
		}

		//Erase from our map
		props.erase(it);
	}
}








