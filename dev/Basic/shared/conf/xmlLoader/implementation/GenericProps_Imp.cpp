#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::generic_props_pimpl::pre ()
{
}

void sim_mob::conf::generic_props_pimpl::post_generic_props ()
{
}

void sim_mob::conf::generic_props_pimpl::property (const std::pair<std::string, std::string>& value)
{
	config->system().genericProps[value.first] = value.second;
}
















