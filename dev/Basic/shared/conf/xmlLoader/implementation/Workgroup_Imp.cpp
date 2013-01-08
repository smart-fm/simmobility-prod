#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::workgroup_pimpl::pre ()
{
}

void sim_mob::conf::workgroup_pimpl::post_workgroup ()
{
}

void sim_mob::conf::workgroup_pimpl::id (const ::std::string& id)
{
  std::cout << "id: " << id << std::endl;
}

void sim_mob::conf::workgroup_pimpl::workers (int workers)
{
  std::cout << "workers: " << workers << std::endl;
}

