#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::TripChains_t_pimpl::pre ()
{
//	model->clear(); //model links to actual repository of tripchains. we dont clear it here as there might be other sources of setting tripchains than the XML
}

void sim_mob::xml::TripChains_t_pimpl::post_TripChains_t ()
{
}

void sim_mob::xml::TripChains_t_pimpl::TripChain (std::pair<std::string, std::vector<sim_mob::TripChainItem*> > value)
{
	model->insert(value);//assigns tripchains to a person(in a std::map). this assignment is final
	//TODO: Avoid static references!
//	(*tripChains)[value.first] = value.second;
}

