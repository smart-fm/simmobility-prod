#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::SimMobility_pimpl::pre ()
{
}

void sim_mob::conf::SimMobility_pimpl::post_SimMobility ()
{
}

void sim_mob::conf::SimMobility_pimpl::constructs ()
{
}

void sim_mob::conf::SimMobility_pimpl::single_threaded (bool single_threaded)
{
  std::cout << "single_threaded: " << single_threaded << std::endl;
}

void sim_mob::conf::SimMobility_pimpl::system ()
{
}

void sim_mob::conf::SimMobility_pimpl::simulation ()
{
}



