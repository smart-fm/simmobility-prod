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
	uniNodes.clear();
	multiNodes.clear();
	Lookup.clear();
}

std::pair< std::set<sim_mob::UniNode*>, std::set<sim_mob::MultiNode*> > sim_mob::xml::Nodes_pimpl::post_Nodes ()
{
	//TODO: There's really no need to make a copy; we've been dealing with references all along.
	//      Perhaps we should find a way to pass "uniNodes" on to the appropriate parser?
	return std::make_pair(uniNodes, multiNodes);
}

void sim_mob::xml::Nodes_pimpl::UniNodes (const std::set<sim_mob::UniNode*>& value)
{
	uniNodes = value;
	//rn.setSegmentNodes(value);
}

void sim_mob::xml::Nodes_pimpl::Intersections (const std::vector<sim_mob::MultiNode*>& value)
{
	multiNodes.insert(value.begin(), value.end());
	//rn.addNodes(value);
}

void sim_mob::xml::Nodes_pimpl::roundabouts (const std::vector<sim_mob::MultiNode*>& value)
{
	multiNodes.insert(value.begin(), value.end());
	//rn.addNodes(value);
}

