#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::database_loader_pimpl::pre ()
{
}

void sim_mob::conf::database_loader_pimpl::post_database_loader ()
{
}

void sim_mob::conf::database_loader_pimpl::connection (const ::std::string& connection)
{
  std::cout << "connection: " << connection << std::endl;
}

void sim_mob::conf::database_loader_pimpl::mappings (const ::std::string& mappings)
{
  std::cout << "mappings: " << mappings << std::endl;
}


