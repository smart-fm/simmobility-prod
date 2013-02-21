#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using sim_mob::StoredProcedureMap;
using std::string;
using std::pair;

void sim_mob::conf::proc_map_pimpl::pre ()
{
}

std::pair<std::string, sim_mob::StoredProcedureMap> sim_mob::conf::proc_map_pimpl::post_proc_map ()
{
	//Input validation
	if (pmFormat!="aimsun") {
		throw std::runtime_error("DB Procedure mappings only support the aimsun format for now.");
	}

	//Prepare a result.
	StoredProcedureMap res(pmID);
	res.dbFormat = pmFormat;
	res.procedureMappings = pmParams;

	//Return
	return std::make_pair(res.getId(), res);
}

void sim_mob::conf::proc_map_pimpl::mapping (const pair<string, string>& value)
{
	pmParams[value.first] = value.second;
}

void sim_mob::conf::proc_map_pimpl::id (const string& value)
{
	pmID = value;
}

void sim_mob::conf::proc_map_pimpl::format (const string& value)
{
	pmFormat = value;
}
