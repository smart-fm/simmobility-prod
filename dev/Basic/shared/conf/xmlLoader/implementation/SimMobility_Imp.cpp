#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

#include "conf/Config.hpp"

using std::string;

void sim_mob::conf::SimMobility_pimpl::pre ()
{
}

void sim_mob::conf::SimMobility_pimpl::post_SimMobility ()
{
}

void sim_mob::conf::SimMobility_pimpl::single_threaded (bool single_threaded)
{
	config->singleThreaded() = single_threaded;
}

