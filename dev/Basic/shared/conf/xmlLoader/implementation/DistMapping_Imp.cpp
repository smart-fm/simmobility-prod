#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::dist_mapping_pimpl::pre ()
{
	model = "";
}

std::string sim_mob::conf::dist_mapping_pimpl::post_dist_mapping ()
{
	return model;
}

void sim_mob::conf::dist_mapping_pimpl::dist (const ::std::string& value)
{
	model = value;
}
