#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


///TODO: Accessing static data this way is bad!
sim_mob::xml::RoadNetwork_t_pimpl::RoadNetwork_t_pimpl() :
	modelRef(sim_mob::ConfigParams::GetInstance().getNetworkRW())
{
}

void sim_mob::xml::RoadNetwork_t_pimpl::post_RoadNetwork_t ()
{
}

void sim_mob::xml::RoadNetwork_t_pimpl::pre ()
{
}

void sim_mob::xml::RoadNetwork_t_pimpl::Nodes (const std::pair< std::set<sim_mob::UniNode*>, std::set<sim_mob::MultiNode*> >& value)
{
	modelRef.segmentnodes = value.first;
	modelRef.nodes.insert(modelRef.nodes.end(), value.second.begin(), value.second.end());

	//TODO: Why isn't this a set?
	//modelRef.nodes = value.second;
}

void sim_mob::xml::RoadNetwork_t_pimpl::Links (const std::vector<sim_mob::Link*>& value)
{
	modelRef.setLinks(value);
}

