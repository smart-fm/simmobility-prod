#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::drivers_pimpl::pre ()
{
}

void sim_mob::conf::drivers_pimpl::post_drivers ()
{
}

void sim_mob::conf::drivers_pimpl::database_loader (const std::pair<std::string, std::string>& value)
{
	config->simulation().agentsLoaders.push_back(new DbLoader(value.first, value.second));
}

void sim_mob::conf::drivers_pimpl::xml_loader (const std::pair<std::string, std::string>& value)
{
	config->simulation().agentsLoaders.push_back(new XmlLoader(value.first, value.second));
}

void sim_mob::conf::drivers_pimpl::driver (const sim_mob::AgentSpec<sim_mob::DriverSpec>& value)
{
	typedef AgentLoader< DriverSpec > DriverLoader;

	//To avoid too much waste, we stack multiple agent definitions in a row onto the same loader.
	std::list<DataLoader*>& loaders = config->simulation().agentsLoaders;
	if (loaders.empty() || !dynamic_cast<DriverLoader*>(loaders.back())) {
		loaders.push_back(new DriverLoader());
	}
	dynamic_cast<DriverLoader*>(loaders.back())->addAgentSpec(value);
}


