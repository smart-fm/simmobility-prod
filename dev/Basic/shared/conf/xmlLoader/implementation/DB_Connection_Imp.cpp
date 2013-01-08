#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;

void sim_mob::conf::db_connection_pimpl::pre ()
{
}

void sim_mob::conf::db_connection_pimpl::post_db_connection ()
{
}

void sim_mob::conf::db_connection_pimpl::param ()
{
}

void sim_mob::conf::db_connection_pimpl::id (const ::std::string& id)
{
  std::cout << "id: " << id << std::endl;
}

void sim_mob::conf::db_connection_pimpl::dbtype (const ::std::string& dbtype)
{
  std::cout << "dbtype: " << dbtype << std::endl;
}

