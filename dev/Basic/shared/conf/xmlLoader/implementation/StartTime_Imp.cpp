#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::start_time_pimpl::pre ()
{
}

void sim_mob::conf::start_time_pimpl::post_start_time ()
{
}

void sim_mob::conf::start_time_pimpl::value (const ::std::string& value)
{
  std::cout << "value: " << value << std::endl;
}


















