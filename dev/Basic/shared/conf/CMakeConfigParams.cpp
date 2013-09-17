//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "CMakeConfigParams.hpp"
#include "GenConfig.h"

using namespace sim_mob;

bool sim_mob::CMakeConfigParams::MPI_Enabled() const
{
	return !MPI_Disabled();
}
bool sim_mob::CMakeConfigParams::MPI_Disabled() const
{
#ifdef SIMMOB_DISABLE_MPI
	return true;
#else
	return false;
#endif
}
// use pathset to generate path of driver
bool sim_mob::CMakeConfigParams::PathSetMode() const {
#ifdef SIMMOB_PATHSET_MODE
	return true;
#else
	return false;
#endif
}
bool sim_mob::CMakeConfigParams::OutputEnabled() const
{
	return !OutputDisabled();
}
bool sim_mob::CMakeConfigParams::OutputDisabled() const
{
#ifdef SIMMOB_DISABLE_OUTPUT
	return true;
#else
	return false;
#endif
}


bool sim_mob::CMakeConfigParams::StrictAgentErrors() const
{
#ifdef SIMMOB_STRICT_AGENT_ERRORS
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfigParams::ProfileOn() const
{
#ifdef SIMMOB_PROFILE_ON
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfigParams::ProfileAgentUpdates(bool accountForOnFlag) const
{
#ifdef SIMMOB_PROFILE_AGENT_UPDATES
	if (accountForOnFlag) {
		return ProfileOn();
	}
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfigParams::ProfileWorkerUpdates(bool accountForOnFlag) const
{
#ifdef SIMMOB_PROFILE_WORKER_UPDATES
	if (accountForOnFlag) {
		return ProfileOn();
	}
	return true;
#else
	return false;
#endif
}


bool sim_mob::CMakeConfigParams::UsingConfluxes() const
{
#ifdef SIMMOB_USE_CONFLUXES
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfigParams::InteractiveMode() const
{
#ifdef SIMMOB_INTERACTIVE_MODE
	return true;
#else
	return false;
#endif
}

bool sim_mob::CMakeConfigParams::XmlWriterOn() const
{
#ifdef SIMMOB_XML_WRITER
	return true;
#else
	return false;
#endif
}


