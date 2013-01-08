#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::db_param_pimpl::pre ()
{
}

void sim_mob::conf::db_param_pimpl::post_db_param ()
{
}

void sim_mob::conf::db_param_pimpl::name (const ::std::string& name)
{
  std::cout << "name: " << name << std::endl;
}

void sim_mob::conf::db_param_pimpl::value (const ::std::string& value)
{
  std::cout << "value: " << value << std::endl;
}

