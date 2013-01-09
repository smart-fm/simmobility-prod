#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::db_proc_mapping_pimpl::pre ()
{
	model = std::make_pair("", "");
}

pair<string, string> sim_mob::conf::db_proc_mapping_pimpl::post_db_proc_mapping ()
{
	return model;
}

void sim_mob::conf::db_proc_mapping_pimpl::name (const ::std::string& value)
{
	model.first = value;
}

void sim_mob::conf::db_proc_mapping_pimpl::procedure (const ::std::string& value)
{
	model.second = value;
}

