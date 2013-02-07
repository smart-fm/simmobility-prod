#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::road_network_pimpl::pre ()
{
}

void sim_mob::conf::road_network_pimpl::post_road_network ()
{
}

void sim_mob::conf::road_network_pimpl::database_loader (const std::pair<std::string, std::string>& value)
{
	config->simulation().roadNetworkLoaders.push_back(new sim_mob::DbLoader(value.first, value.second));
}

void sim_mob::conf::road_network_pimpl::xml_loader (const std::pair<std::string, std::string>& value)
{
	config->simulation().roadNetworkLoaders.push_back(new sim_mob::XmlLoader(value.first, value.second));
}



