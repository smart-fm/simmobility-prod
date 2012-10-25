#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


//TODO: Using a static field like this means we can't support multi-threaded loading of different networks.
//      It shouldn't be too hard to add some "temporary" object at a scope global to the parser classes. ~Seth
std::map<unsigned int,sim_mob::Node*> sim_mob::xml::Nodes_pimpl::Lookup;

//Functionality for retrieving a Node from the list of known Nodes.
sim_mob::Node* sim_mob::xml::Nodes_pimpl::LookupNode(unsigned int id) {
	std::map<unsigned int,sim_mob::Node*>::iterator it = Lookup.find(id);
	if (it!=Lookup.end()) {
		return it->second;
	}
	throw std::runtime_error("Unknown Node id.");
}

//Functionality for registering a Node so that it can be retrieved later.
void sim_mob::xml::Nodes_pimpl::RegisterNode(unsigned int id, sim_mob::Node* node) {
	if (Lookup.count(id)>0) {
		throw std::runtime_error("Node id is already registered.");
	}
	Lookup[id] = node;
}


void sim_mob::xml::Nodes_pimpl::pre ()
{
	//Clear it in case post() wasn't called.
	Lookup.clear();
}

void sim_mob::xml::Nodes_pimpl::post_Nodes ()
{
	//Clear it to save memory.
	Lookup.clear();
}

void sim_mob::xml::Nodes_pimpl::UniNodes (std::set<sim_mob::UniNode*>& value)
{
	rn.setSegmentNodes(value);
}

void sim_mob::xml::Nodes_pimpl::Intersections (std::vector<sim_mob::MultiNode*>& value)
{
	rn.addNodes(value);
}

void sim_mob::xml::Nodes_pimpl::roundabouts (std::vector<sim_mob::MultiNode*>& value)
{
	rn.addNodes(value);
}

