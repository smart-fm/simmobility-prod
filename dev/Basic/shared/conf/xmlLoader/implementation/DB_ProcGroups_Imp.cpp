#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::db_proc_groups_pimpl::pre ()
{
}

void sim_mob::conf::db_proc_groups_pimpl::post_db_proc_groups ()
{
}

void sim_mob::conf::db_proc_groups_pimpl::proc_map (const std::pair<std::string, sim_mob::StoredProcedureMap>& value)
{
	//config->constructs().storedProcedureMaps[value.first] = value.second;
}











