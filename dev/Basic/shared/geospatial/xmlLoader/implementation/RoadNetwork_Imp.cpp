#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


sim_mob::RoadNetwork& sim_mob::xml::RoadNetwork_t_pimpl::post_RoadNetwork_t ()
{
	return modelRef;
}

void sim_mob::xml::RoadNetwork_t_pimpl::pre ()
{
}

void sim_mob::xml::RoadNetwork_t_pimpl::Nodes (const helper::NodesRes& value)
{
	modelRef.segmentnodes = value.uniNodes;
	modelRef.nodes.insert(modelRef.nodes.end(), value.multiNodes.begin(), value.multiNodes.end());

	//TODO: Why isn't this a set?
	//modelRef.nodes = value.second;
}

void sim_mob::xml::RoadNetwork_t_pimpl::Links (const std::vector<sim_mob::Link*>& value)
{
	modelRef.setLinks(value);
}

