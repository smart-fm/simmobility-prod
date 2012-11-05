#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;



void sim_mob::xml::Links_pimpl::pre ()
{
	//Clear it in case post() wasn't called.
	//Lookup.clear();
}

void sim_mob::xml::Links_pimpl::Link (sim_mob::Link* Link)
{
	model.push_back(Link);
}

std::vector<sim_mob::Link*> Links_pimpl::post_Links ()
{
	for (std::vector<sim_mob::Link*>::iterator it=model.begin(); it!=model.end(); it++) {
		book.addLink(*it);
	}

	  return model;
}

