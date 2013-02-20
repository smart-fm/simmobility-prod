#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::pedestrians_pimpl::pre ()
{
}

void sim_mob::conf::pedestrians_pimpl::post_pedestrians ()
{
}

void sim_mob::conf::pedestrians_pimpl::database_loader (const std::pair<std::string, std::string>& value)
{
	config->simulation().agentsLoaders.push_back(new DatabaseAgentLoader(value.first, value.second));
}

void sim_mob::conf::pedestrians_pimpl::xml_loader (const std::pair<std::string, std::string>& value)
{
	config->simulation().agentsLoaders.push_back(new XmlAgentLoader(value.first, value.second));
}

void sim_mob::conf::pedestrians_pimpl::pedestrian (const sim_mob::AgentSpec& value)
{
	//To avoid too much waste, we stack multiple agent definitions in a row onto the same loader.
	std::list<AbstractAgentLoader*>& loaders = config->simulation().agentsLoaders;
	if (loaders.empty() || !dynamic_cast<AgentLoader*>(loaders.back())) {
		loaders.push_back(new AgentLoader());
	}
	dynamic_cast<AgentLoader*>(loaders.back())->addAgentSpec(value);
}




