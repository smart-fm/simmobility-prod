#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::val_units_pimpl::pre ()
{
}

void sim_mob::conf::val_units_pimpl::post_val_units ()
{
}

void sim_mob::conf::val_units_pimpl::value (int value)
{
  std::cout << "value: " << value << std::endl;
}

void sim_mob::conf::val_units_pimpl::units (const ::std::string& units)
{
  std::cout << "units: " << units << std::endl;
}

