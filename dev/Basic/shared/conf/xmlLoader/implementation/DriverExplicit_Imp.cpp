#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::driver_explicit_pimpl::pre ()
{
}

void sim_mob::conf::driver_explicit_pimpl::post_driver_explicit ()
{
}

void sim_mob::conf::driver_explicit_pimpl::property ()
{
}

void sim_mob::conf::driver_explicit_pimpl::originPos (const ::std::string& originPos)
{
  std::cout << "originPos: " << originPos << std::endl;
}

void sim_mob::conf::driver_explicit_pimpl::destPos (const ::std::string& destPos)
{
  std::cout << "destPos: " << destPos << std::endl;
}

void sim_mob::conf::driver_explicit_pimpl::startTime (const ::std::string& startTime)
{
  std::cout << "startTime: " << startTime << std::endl;
}

void sim_mob::conf::driver_explicit_pimpl::startFrame (int startFrame)
{
  std::cout << "startFrame: " << startFrame << std::endl;
}
