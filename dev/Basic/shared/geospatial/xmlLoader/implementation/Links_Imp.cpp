#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


//TODO: Using a static field like this means we can't support multi-threaded loading of different networks.
//      It shouldn't be too hard to add some "temporary" object at a scope global to the parser classes. ~Seth
std::map<unsigned int,sim_mob::Link*> sim_mob::xml::Links_pimpl::Lookup;

//Functionality for retrieving a Link from the list of known Links.
sim_mob::Link* sim_mob::xml::Links_pimpl::LookupLink(unsigned int id) {
	std::map<unsigned int,sim_mob::Link*>::iterator it = Lookup.find(id);
	if (it!=Lookup.end()) {
		return it->second;
	}
	throw std::runtime_error("Unknown Link id.");
}

//Functionality for registering a Link so that it can be retrieved later.
void sim_mob::xml::Links_pimpl::RegisterLink(unsigned int id, sim_mob::Link* link) {
	if (Lookup.count(id)>0) {
		throw std::runtime_error("Link id is already registered.");
	}
	Lookup[id] = link;
}


void sim_mob::xml::Links_pimpl::pre ()
{
	//Clear it in case post() wasn't called.
	Lookup.clear();
}

void sim_mob::xml::Links_pimpl::Link (sim_mob::Link* Link)
{
	model.push_back(Link);
}

std::vector<sim_mob::Link*> Links_pimpl::post_Links ()
{
	  return model;
}

