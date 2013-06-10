#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::TripChain_t_pimpl::pre ()
{
	model.first = -1;
	model.second.clear();
}

std::pair<std::string, std::vector<sim_mob::TripChainItem*> > sim_mob::xml::TripChain_t_pimpl::post_TripChain_t ()
{
	return model;
}

void sim_mob::xml::TripChain_t_pimpl::personID (const ::std::string& value)
{
	model.first = value;
}

void sim_mob::xml::TripChain_t_pimpl::Trip (sim_mob::TripChainItem* value)
{
	model.second.push_back(value);
}

void sim_mob::xml::TripChain_t_pimpl::Activity (sim_mob::TripChainItem* value)
{
	model.second.push_back(value);
}

