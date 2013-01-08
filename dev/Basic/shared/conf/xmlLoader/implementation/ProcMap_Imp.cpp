#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::proc_map_pimpl::pre ()
{
}

void sim_mob::conf::proc_map_pimpl::post_proc_map ()
{
}

void sim_mob::conf::proc_map_pimpl::mapping ()
{
}

void sim_mob::conf::proc_map_pimpl::id (const ::std::string& id)
{
  std::cout << "id: " << id << std::endl;
}

void sim_mob::conf::proc_map_pimpl::format (const ::std::string& format)
{
  std::cout << "format: " << format << std::endl;
}
