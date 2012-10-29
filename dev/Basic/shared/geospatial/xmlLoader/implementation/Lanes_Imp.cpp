#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


//TODO: Using a static field like this means we can't support multi-threaded loading of different networks.
//      It shouldn't be too hard to add some "temporary" object at a scope global to the parser classes. ~Seth
std::map<unsigned int,sim_mob::Lane*> sim_mob::xml::Lanes_pimpl::Lookup;

//Functionality for retrieving a Lane from the list of known Lanes.
sim_mob::Lane* sim_mob::xml::Lanes_pimpl::LookupLane(unsigned int id) {
	std::map<unsigned int,sim_mob::Lane*>::iterator it = Lookup.find(id);
	if (it!=Lookup.end()) {
		return it->second;
	}
	throw std::runtime_error("Unknown Lane id.");
}

//Functionality for registering a Node so that it can be retrieved later.
void sim_mob::xml::Lanes_pimpl::RegisterLane(unsigned int id, sim_mob::Lane* node) {
	if (Lookup.count(id)>0) {
		throw std::runtime_error("Lane id is already registered.");
	}
	Lookup[id] = lane;
}

void sim_mob::xml::Lanes_pimpl::pre ()
{
	Lookup.clear();
	model.clear();
}

void sim_mob::xml::Lanes_pimpl::Lane (sim_mob::Lane* value)
{
	model.push_back(value);
}

std::vector<sim_mob::Lane*> sim_mob::xml::Lanes_pimpl::post_Lanes ()
{
	return model;
}
