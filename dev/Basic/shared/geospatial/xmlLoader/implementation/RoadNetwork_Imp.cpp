#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::RoadNetwork_t_pimpl::pre ()
{
}


sim_mob::RoadNetwork& sim_mob::xml::RoadNetwork_t_pimpl::post_RoadNetwork_t ()
{
	throw_if_null();
	return *modelRef;
}


void sim_mob::xml::RoadNetwork_t_pimpl::coordinate_map (const std::vector<sim_mob::CoordinateTransform*>& value)
{
	throw_if_null();
	modelRef->coordinateMap = value;
}

void sim_mob::xml::RoadNetwork_t_pimpl::roadrunner_regions (const std::map<int, sim_mob::RoadRunnerRegion>& value)
{
	throw_if_null();
	modelRef->roadRunnerRegions = value;
}



void sim_mob::xml::RoadNetwork_t_pimpl::Nodes (const helper::NodesRes& value)
{
	throw_if_null();
	modelRef->segmentnodes = value.uniNodes;
	modelRef->nodes.insert(modelRef->nodes.end(), value.multiNodes.begin(), value.multiNodes.end());

	//TODO: Why isn't this a set?
	//modelRef.nodes = value.second;
}

void sim_mob::xml::RoadNetwork_t_pimpl::Links (const std::vector<sim_mob::Link*>& value)
{
	throw_if_null();
	modelRef->setLinks(value);
}

