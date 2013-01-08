#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::model_pimpl::pre ()
{
	model = std::make_pair("", "");
}

pair<string, string> sim_mob::conf::model_pimpl::post_model ()
{
	return model;
}

void sim_mob::conf::model_pimpl::id (const ::std::string& value)
{
	model.first = value;
}

void sim_mob::conf::model_pimpl::library (const ::std::string& value)
{
	if (value!="built-in") { throw std::runtime_error("Only built-in libraries supported (for now)."); }
	model.second = value;
}

