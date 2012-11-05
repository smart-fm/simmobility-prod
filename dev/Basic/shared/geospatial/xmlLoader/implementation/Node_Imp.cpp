#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


//std::map<sim_mob::Node*, unsigned int> sim_mob::xml::Node_t_pimpl::LinkLocCache;


void sim_mob::xml::Node_t_pimpl::pre ()
{
	model = sim_mob::Node(0, 0);
	linkLocSaved = 0;
}

sim_mob::Node sim_mob::xml::Node_t_pimpl::post_Node_t ()
{
	  return model;
}

/*void sim_mob::xml::Node_t_pimpl::RegisterLinkLoc(sim_mob::Node* node, unsigned int link)
{
	if (LinkLocCache.count(node)>0) {
		throw std::runtime_error("Node link id already registered.");
	}
	LinkLocCache[node] = link;
}

unsigned int sim_mob::xml::Node_t_pimpl::GetLinkLoc(sim_mob::Node* node)
{
	std::map<sim_mob::Node*, unsigned int>::iterator it = LinkLocCache.find(node);
	if (it!=LinkLocCache.end()) {
		return it->second;
	}
	throw std::runtime_error("No LinkLoc id saved");
}

std::map<sim_mob::Node*, unsigned int>& sim_mob::xml::Node_t_pimpl::GetLinkLocList()
{
	return LinkLocCache;
}*/




void sim_mob::xml::Node_t_pimpl::nodeID (unsigned int value)
{
	model.nodeId = value;
}

void sim_mob::xml::Node_t_pimpl::location (sim_mob::Point2D value)
{
	model.location = value;
}

void sim_mob::xml::Node_t_pimpl::linkLoc (unsigned long long value)
{
	linkLocSaved = value;
}

void sim_mob::xml::Node_t_pimpl::originalDB_ID (const std::string& value)
{
	model.originalDB_ID = value;
}

