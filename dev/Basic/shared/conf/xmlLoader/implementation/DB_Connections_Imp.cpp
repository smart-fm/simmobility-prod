#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::db_connections_pimpl::pre ()
{
}

void sim_mob::conf::db_connections_pimpl::post_db_connections ()
{
}

void sim_mob::conf::db_connections_pimpl::connection (const std::pair<std::string, sim_mob::DatabaseConnection>& value)
{
	config->constructs().dbConnections[value.first] = value.second;
}









