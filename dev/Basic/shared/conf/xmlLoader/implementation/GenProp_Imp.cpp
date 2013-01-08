#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::gen_prop_pimpl::pre ()
{
}

void sim_mob::conf::gen_prop_pimpl::post_gen_prop ()
{
}

void sim_mob::conf::gen_prop_pimpl::key (const ::std::string& key)
{
  std::cout << "key: " << key << std::endl;
}

void sim_mob::conf::gen_prop_pimpl::value (const ::std::string& value)
{
  std::cout << "value: " << value << std::endl;
}



