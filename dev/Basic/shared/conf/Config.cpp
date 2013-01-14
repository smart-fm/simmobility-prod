/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch
#include "Config.hpp"

#include "GenConfig.h"

#include "workers/WorkGroup.hpp"
#include "partitions/PartitionManager.hpp"
#include "entities/AuraManager.hpp"

using std::string;
using std::map;

using namespace sim_mob;


Config sim_mob::Config::instance_;


sim_mob::Granularity::Granularity(int amount, const string& units)
{
	if (units=="hours") {
		ms_ = amount * 3600000;
	} else if (units=="minutes" || units=="min") {
		ms_ = amount * 60000;
	} else if (units=="seconds" || units=="s") {
		ms_ = amount * 1000;
	} else if (units=="ms") {
		ms_ = amount;
	} else {
		throw std::runtime_error("Unknown units for Granularity.");
	}
}

sim_mob::WorkGroup* WorkGroupFactory::getItem()
{
	if (!item) {
		//Sanity check.
		if (agentWG==signalWG) { throw std::runtime_error("agentWG and signalWG should be mutually exclusive."); }

		//Create it. For now, this is bound to the old "ConfigParams" structure; we can change this later once both files build in parallel.
		const ConfigParams& cf = ConfigParams::GetInstance();
		PartitionManager* partMgr = nullptr;
		if (!cf.MPI_Disabled() && cf.is_run_on_many_computers) {
			partMgr = &PartitionManager::instance();
		}
		if (agentWG) {
			item = WorkGroup::NewWorkGroup(numWorkers, cf.totalRuntimeTicks, cf.granAgentsTicks, &AuraManager::instance(), partMgr);
		} else {
			item = WorkGroup::NewWorkGroup(numWorkers, cf.totalRuntimeTicks, cf.granSignalsTicks);
		}
	}
	return item;
}


const Config& sim_mob::Config::GetInstance()
{
	return instance_;
}

Config& sim_mob::Config::GetInstanceRW()
{
	return instance_;
}

void sim_mob::Config::InitBuiltInModels(const BuiltInModels& models)
{
	built_in_models = models;
}


bool sim_mob::CMakeConfig::MPI_Enabled() const
{
	return !MPI_Disabled();
}
bool sim_mob::CMakeConfig::MPI_Disabled() const
{
#ifdef SIMMOB_DISABLE_MPI
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfig::OutputEnabled() const
{
	return !OutputDisabled();
}
bool sim_mob::CMakeConfig::OutputDisabled() const
{
#ifdef SIMMOB_DISABLE_OUTPUT
	return true;
#else
	return false;
#endif
}


bool sim_mob::CMakeConfig::StrictAgentErrors() const
{
#ifdef SIMMOB_STRICT_AGENT_ERRORS
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfig::GenerateAgentUpdateProfile() const
{
#ifdef SIMMOB_AGENT_UPDATE_PROFILE
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfig::NewSignalModelEnabled() const
{
#ifdef SIMMOB_NEW_SIGNAL
	return true;
#else
	return false;
#endif
}



