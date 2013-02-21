#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::default_model_pimpl::pre ()
{
	model = std::make_pair("", "");
}

std::pair<std::string, std::string> sim_mob::conf::default_model_pimpl::post_default_model ()
{
	return model;
}

void sim_mob::conf::default_model_pimpl::type (const ::std::string& value)
{
	model.first = value;
}

void sim_mob::conf::default_model_pimpl::default_ (const ::std::string& value)
{
	model.second = value;
}

