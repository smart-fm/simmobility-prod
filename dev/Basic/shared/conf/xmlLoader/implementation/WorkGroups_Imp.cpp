#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::workgroups_pimpl::pre ()
{
}

void sim_mob::conf::workgroups_pimpl::post_workgroups ()
{
}

void sim_mob::conf::workgroups_pimpl::workgroup (const std::pair<std::string, sim_mob::WorkGroupFactory>& value)
{
	config->constructs().workGroups[value.first] = value.second;
}





