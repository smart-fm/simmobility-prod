#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::db_proc_mapping_pimpl::pre ()
{
}

void sim_mob::conf::db_proc_mapping_pimpl::post_db_proc_mapping ()
{
}

void sim_mob::conf::db_proc_mapping_pimpl::name (const ::std::string& name)
{
  std::cout << "name: " << name << std::endl;
}

void sim_mob::conf::db_proc_mapping_pimpl::procedure (const ::std::string& procedure)
{
  std::cout << "procedure: " << procedure << std::endl;
}

