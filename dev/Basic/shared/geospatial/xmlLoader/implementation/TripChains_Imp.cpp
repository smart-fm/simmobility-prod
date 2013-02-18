#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::TripChains_t_pimpl::pre ()
{
}

void sim_mob::xml::TripChains_t_pimpl::post_TripChains_t ()
{
}

void sim_mob::xml::TripChains_t_pimpl::TripChain (std::pair<unsigned long, std::vector<sim_mob::TripChainItem*> > value)
{
	//TODO: Avoid static references!
	tripChains[value.first] = value.second;
}

