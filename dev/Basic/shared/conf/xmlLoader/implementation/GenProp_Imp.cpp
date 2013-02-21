#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::gen_prop_pimpl::pre ()
{
	model = std::make_pair("", "");
}

std::pair<std::string, std::string> sim_mob::conf::gen_prop_pimpl::post_gen_prop ()
{
	return model;
}

void sim_mob::conf::gen_prop_pimpl::key (const ::std::string& value)
{
	model.first = value;
}

void sim_mob::conf::gen_prop_pimpl::value (const ::std::string& value)
{
	model.second = value;
}



