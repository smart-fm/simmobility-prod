#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


///TODO: Accessing static data this way is bad!
/*sim_mob::xml::RoadNetwork_t_pimpl::RoadNetwork_t_pimpl() :
	modelRef(sim_mob::ConfigParams::GetInstance().getNetworkRW())
{
	//TODO: Is it safe to put this here?
	//NOTE: This breaks encapsulation, since we'd have to cast to the _imp child class. There's probably a better way.
	//Nodes_parser_->setNodesArray(). ...or something?
}*/

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

