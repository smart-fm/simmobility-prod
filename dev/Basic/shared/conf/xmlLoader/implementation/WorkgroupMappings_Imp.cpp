#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::workgroup_mappings_pimpl::pre ()
{
}

void sim_mob::conf::workgroup_mappings_pimpl::post_workgroup_mappings ()
{
}

void sim_mob::conf::workgroup_mappings_pimpl::agents (const std::string& value)
{
	config->system().defaultWorkGroups.agentWG.name = value;
}

void sim_mob::conf::workgroup_mappings_pimpl::signals (const std::string& value)
{
	config->system().defaultWorkGroups.signalWG.name = value;
}
















