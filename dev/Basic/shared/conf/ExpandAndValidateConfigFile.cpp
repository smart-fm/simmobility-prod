/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "ExpandAndValidateConfigFile.hpp"

#include "conf/simpleconf.hpp"
#include "entities/Agent.hpp"
#include "util/ReactionTimeDistributions.hpp"
#include "workers/Worker.hpp"

using namespace sim_mob;


namespace {

ReactionTimeDist* GenerateReactionTimeDistribution(SimulationParams::ReactionTimeDistDescription rdist)  {
	if (rdist.typeId==0) {
		return new NormalReactionTimeDist(rdist.mean, rdist.stdev);
	} else if (rdist.typeId==1) {
		return new LognormalReactionTimeDist(rdist.mean, rdist.stdev);
	} else {
		throw std::runtime_error("Unknown reaction time magic number.");
	}
}

void InformLoadOrder(const std::vector<SimulationParams::LoadAgentsOrderOption>& order) {
	std::cout <<"Agent Load order: ";
	if (order.empty()) {
		std::cout <<"<N/A>";
	} else {
		for (std::vector<SimulationParams::LoadAgentsOrderOption>::const_iterator it=order.begin(); it!=order.end(); it++) {
			if ((*it)==SimulationParams::LoadAg_Drivers) {
				std::cout <<"drivers";
			} else if ((*it)==SimulationParams::LoadAg_Database) {
				std::cout <<"database";
			} else if ((*it)==SimulationParams::LoadAg_Pedestrians) {
				std::cout <<"pedestrians";
			} else {
				std::cout <<"<unknown>";
			}
			std::cout <<"  ";
		}
	}
	std::cout <<std::endl;
}

} //End un-named namespace


sim_mob::ExpandAndValidateConfigFile::ExpandAndValidateConfigFile(ConfigParams& result) : cfg(result)
{
	ProcessConfig();
}

void sim_mob::ExpandAndValidateConfigFile::ProcessConfig()
{
	//Set reaction time distributions
	//TODO: Refactor to avoid magic numbers
	cfg.reactDist1 = GenerateReactionTimeDistribution(cfg.system.simulation.reactTimeDistribution1);
	cfg.reactDist2 = GenerateReactionTimeDistribution(cfg.system.simulation.reactTimeDistribution2);

	//Inform of load order (drivers, database, pedestrians, etc.).
	InformLoadOrder(cfg.system.simulation.loadAgentsOrder);

	//Set the auto-incrementing ID.
	if (cfg.system.simulation.startingAutoAgentID<=0) {
		throw std::runtime_error("Agent auto-id must start from >0.");
	}
	Agent::SetIncrementIDStartValue(cfg.system.simulation.startingAutoAgentID, true);

	//Print schema file.
	const std::string schem = cfg.roadNetworkXsdSchemaFile();
	Print() <<"XML (road network) schema file: "  <<(schem.empty()?"<default>":schem) <<std::endl;

	//Ensure granularities are multiples of each other.
	CheckGranularities();


	throw 1;
}


void sim_mob::ExpandAndValidateConfigFile::CheckGranularities()
{
    //Granularity check
	const unsigned int baseGranMS = cfg.system.simulation.baseGranMS;
	const WorkerParams& workers = cfg.system.workers;

    if (workers.person.granularityMs < baseGranMS) {
    	throw std::runtime_error("Person granularity cannot be smaller than base granularity.");
    }
    if (workers.person.granularityMs%baseGranMS != 0) {
    	throw std::runtime_error("Person granularity not a multiple of base granularity.");
    }
    if (workers.signal.granularityMs < baseGranMS) {
    	throw std::runtime_error("Signal granularity cannot be smaller than base granularity.");
    }
    if (workers.signal.granularityMs%baseGranMS != 0) {
    	throw std::runtime_error("Signal granularity not a multiple of base granularity.");
    }
    if (workers.communication.granularityMs < baseGranMS) {
    	throw std::runtime_error("Communication granularity cannot be smaller than base granularity.");
    }
    if (workers.communication.granularityMs%baseGranMS != 0) {
    	throw std::runtime_error("Communication granularity not a multiple of base granularity.");
    }
    if (cfg.system.simulation.totalRuntimeMS < baseGranMS) {
    	throw std::runtime_error("Total Runtime cannot be smaller than base granularity.");
    }
    if (cfg.system.simulation.totalRuntimeMS%baseGranMS != 0) {
    	Warn() <<"Total runtime (" <<cfg.system.simulation.totalRuntimeMS <<") will be truncated by the base granularity (" <<baseGranMS <<")\n";
    }
    if (cfg.system.simulation.totalWarmupMS != 0 && cfg.system.simulation.totalWarmupMS < baseGranMS) {
    	Warn() << "Warning! Total Warmup is smaller than base granularity.\n";
    }
    if (cfg.system.simulation.totalWarmupMS%baseGranMS != 0) {
    	Warn() <<"Total warmup (" <<cfg.system.simulation.totalWarmupMS <<") will be truncated by the base granularity (" <<baseGranMS <<")\n";
    }
}



