#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::TripChains_t_pimpl::pre ()
{
}

void sim_mob::xml::TripChains_t_pimpl::post_TripChains_t ()
{
}

void sim_mob::xml::TripChains_t_pimpl::TripChain (std::pair<std::string, std::vector<sim_mob::TripChainItem*> > value)
{
	//TODO: Avoid static references!
	sim_mob::ConfigParams::GetInstance().getTripChains()[value.first] = value.second;
}

