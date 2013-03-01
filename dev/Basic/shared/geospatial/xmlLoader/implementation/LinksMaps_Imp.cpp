#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;



void sim_mob::xml::links_maps_t_pimpl::pre ()
{
	model.clear();
}

void sim_mob::xml::links_maps_t_pimpl::links_map (std::pair<sim_mob::Link*,sim_mob::linkToLink> value)
{
	model.insert(value);
}

std::multimap<sim_mob::Link*,sim_mob::linkToLink> sim_mob::xml::links_maps_t_pimpl::post_links_maps_t ()
{
	return model;
}
