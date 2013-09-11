#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::distributions_pimpl::pre ()
{
}

void sim_mob::conf::distributions_pimpl::post_distributions ()
{
}

void sim_mob::conf::distributions_pimpl::dist (const std::pair<std::string, sim_mob::ReactionTimeDist*>& value)
{
	//config->constructs().distributions[value.first] = value.second;
}







