#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::db_param_pimpl::pre ()
{
	model = std::make_pair("", "");
}

pair<string, string> sim_mob::conf::db_param_pimpl::post_db_param ()
{
	return model;
}

void sim_mob::conf::db_param_pimpl::name (const ::std::string& value)
{
	model.first = value;
}

void sim_mob::conf::db_param_pimpl::value (const ::std::string& value)
{
	model.second = value;
}

