/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch
#include "Config.hpp"

#include "GenConfig.h"

using namespace sim_mob;


Config sim_mob::Config::instance_;


Config& sim_mob::Config::GetInstance()
{
	return instance_;
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



