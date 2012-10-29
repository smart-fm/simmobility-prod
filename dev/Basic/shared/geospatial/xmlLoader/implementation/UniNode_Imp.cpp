#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;

//TODO: Using a static field like this means we can't support multi-threaded loading of different networks.
//      It shouldn't be too hard to add some "temporary" object at a scope global to the parser classes. ~Seth
std::map<sim_mob::UniNode*, UniNode_t_pimpl::LaneConnectSet> sim_mob::xml::UniNode_t_pimpl::ConnectCache;

//Register a set of connectors for retrieval later.
void sim_mob::xml::UniNode_t_pimpl::RegisterConnectors(sim_mob::UniNode* lane, const UniNode_t_pimpl::LaneConnectSet& connectors)
{
	if (ConnectCache.count(lane)>0) {
		throw std::runtime_error("UniNode connectors are already registered.");
	}
	ConnectCache[lane] = connectors;
}


UniNode_t_pimpl::LaneConnectSet sim_mob::xml::UniNode_t_pimpl::GetConnectors(sim_mob::UniNode* lane)
{
	std::map<sim_mob::UniNode*, UniNode_t_pimpl::LaneConnectorSet>::iterator it = ConnectCache.find(lane);
	if (it!=ConnectCache.end()) {
		return it->second;
	}
	return UniNode_t_pimpl::LaneConnectSet(); //Just return an empty set; there may be no connectors.
}




void sim_mob::xml::UniNode_t_pimpl::pre ()
{
	Node_t_pimpl::pre();
	model = sim_mob::UniNode();
}

sim_mob::UniNode* sim_mob::xml::UniNode_t_pimpl::post_UniNode_t ()
{
	sim_mob::UniNode* res = new sim_mob::UniNode(model);

	//Save the set of connectors for later, since we can't construct it until Lanes have been loaded.
	UniNode_t_pimpl::RegisterConnectors(res, connectors);


	//NOTE: This retrieves the parent Node*, but it also allocates it.
	//sim_mob::Node* v (post_Node_t ());


	  this->uniNode = new sim_mob::UniNode(v->getLocation().getX(), v->getLocation().getY());
	  this->uniNode->setID(v->getID());
	  this->uniNode->originalDB_ID = v->originalDB_ID;
	  geo_UniNodeConnectorsMap[this->uniNode->getID()] = connectors_;

	 // geo_Nodes_[this->uniNode->getID()] = this->uniNode;
	  Nodes_pimpl::RegisterNode(this->uniNode->getID(), this->uniNode);

	  geo_LinkLoc_rawNode & container = get<2>(geo_LinkLoc_);
	  geo_LinkLoc_rawNode::iterator it = container.find(v);
	  if(it != container.end())
	   it->node.push_back(this->uniNode);
	  geo_UniNode_SegmentPairs[this->uniNode->getID()] = std::make_pair(firstSegmentPair, secondSegmentPair);


	  return res;
}

void sim_mob::xml::UniNode_t_pimpl::firstPair (std::pair<unsigned long,unsigned long> value)
{
	model.firstPair = value;
}

void sim_mob::xml::UniNode_t_pimpl::secondPair (std::pair<unsigned long,unsigned long> value)
{
	model.secondPair = value;
}

void sim_mob::xml::UniNode_t_pimpl::Connectors (std::set<std::pair<unsigned long,unsigned long> > value)
{
	//Save for later.
	connectors = value;
}


