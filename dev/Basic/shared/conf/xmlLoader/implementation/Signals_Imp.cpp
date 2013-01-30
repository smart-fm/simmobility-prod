#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::signals_pimpl::pre ()
{
}

void sim_mob::conf::signals_pimpl::post_signals ()
{
}

void sim_mob::conf::signals_pimpl::database_loader (const std::pair<std::string, std::string>& value)
{
	config->simulation().agentsLoaders.push_back(new DbLoader(value.first, value.second));
}

void sim_mob::conf::signals_pimpl::xml_loader (const std::pair<std::string, std::string>& value)
{
	config->simulation().agentsLoaders.push_back(new XmlLoader(value.first, value.second));
}




