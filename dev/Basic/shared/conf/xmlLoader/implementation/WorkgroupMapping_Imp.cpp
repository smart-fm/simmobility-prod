#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::workgroup_mapping_pimpl::pre ()
{
}

void sim_mob::conf::workgroup_mapping_pimpl::post_workgroup_mapping ()
{
}

void sim_mob::conf::workgroup_mapping_pimpl::workgroup (const ::std::string& workgroup)
{
  std::cout << "workgroup: " << workgroup << std::endl;
}


