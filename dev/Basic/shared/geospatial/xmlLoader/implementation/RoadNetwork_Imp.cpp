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

void sim_mob::xml::RoadNetwork_t_pimpl::Nodes ()
{
}

void sim_mob::xml::RoadNetwork_t_pimpl::Links (std::vector<sim_mob::Link*> value)
{
	modelRef.setLinks(value);
}

