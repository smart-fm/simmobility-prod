#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;

//TODO: Using a static field like this means we can't support multi-threaded loading of different networks.
//      It shouldn't be too hard to add some "temporary" object at a scope global to the parser classes. ~Seth
std::map<sim_mob::UniNode*, UniNode_t_pimpl::LaneConnectSet> sim_mob::xml::UniNode_t_pimpl::ConnectCache;
std::map<sim_mob::UniNode*, std::pair<UniNode_t_pimpl::SegmentPair, UniNode_t_pimpl::SegmentPair> > sim_mob::xml::UniNode_t_pimpl::SegmentPairCache;

//Register a set of connectors for retrieval later.
void sim_mob::xml::UniNode_t_pimpl::RegisterConnectors(sim_mob::UniNode* node, const UniNode_t_pimpl::LaneConnectSet& connectors)
{
	if (ConnectCache.count(node)>0) {
		throw std::runtime_error("UniNode connectors are already registered.");
	}
	ConnectCache[node] = connectors;
}


UniNode_t_pimpl::LaneConnectSet sim_mob::xml::UniNode_t_pimpl::GetConnectors(sim_mob::UniNode* node)
{
	std::map<sim_mob::UniNode*, UniNode_t_pimpl::LaneConnectSet>::iterator it = ConnectCache.find(node);
	if (it!=ConnectCache.end()) {
		return it->second;
	}
	return UniNode_t_pimpl::LaneConnectSet(); //Just return an empty set; there may be no connectors.
}


void sim_mob::xml::UniNode_t_pimpl::RegisterSegmentPairs(sim_mob::UniNode* node, const std::pair<UniNode_t_pimpl::SegmentPair, UniNode_t_pimpl::SegmentPair>& pairs)
{
	if (SegmentPairCache.count(node)>0) {
		throw std::runtime_error("UniNode segments are already registered.");
	}
	SegmentPairCache[node] = pairs;
}


std::pair<UniNode_t_pimpl::SegmentPair, UniNode_t_pimpl::SegmentPair> sim_mob::xml::UniNode_t_pimpl::GetSegmentPairs(sim_mob::UniNode* node)
{
	std::map<sim_mob::UniNode*, std::pair<UniNode_t_pimpl::SegmentPair, UniNode_t_pimpl::SegmentPair> >::iterator it = SegmentPairCache.find(node);
	if (it!=SegmentPairCache.end()) {
		return it->second;
	}
	return std::pair<UniNode_t_pimpl::SegmentPair, UniNode_t_pimpl::SegmentPair>(); //Just return an empty set; there may be no connectors.
}



void sim_mob::xml::UniNode_t_pimpl::pre ()
{
	Node_t_pimpl::pre();
	model = sim_mob::UniNode(0,0);
	connectors.clear();
	segmentPairs = std::make_pair(SegmentPair(), SegmentPair());
}

sim_mob::UniNode* sim_mob::xml::UniNode_t_pimpl::post_UniNode_t ()
{
	sim_mob::UniNode* res = new sim_mob::UniNode(model);

	//Save the set of connectors for later, since we can't construct it until Lanes have been loaded.
	UniNode_t_pimpl::RegisterConnectors(res, connectors);

	//NOTE: This retrieves the parent Node*, but it also allocates it. Replace it as a value type return if possible.
	sim_mob::Node tempNode = Node_t_pimpl::post_Node_t();
	res->location = sim_mob::Point2D(tempNode.getLocation());
	res->setID(tempNode.getID());
	res->originalDB_ID = tempNode.originalDB_ID;

	//TODO: Make sure these are all covered.
	/*geo_LinkLoc_rawNode & container = get<2>(geo_LinkLoc_);
	geo_LinkLoc_rawNode::iterator it = container.find(v);
	if(it != container.end()) {
		it->node.push_back(this->uniNode);
	}
	geo_UniNode_SegmentPairs[this->uniNode->getID()] = std::make_pair(firstSegmentPair, secondSegmentPair);*/

	return res;
}

void sim_mob::xml::UniNode_t_pimpl::firstPair (std::pair<unsigned long,unsigned long> value)
{
	segmentPairs.first = value;
}

void sim_mob::xml::UniNode_t_pimpl::secondPair (std::pair<unsigned long,unsigned long> value)
{
	segmentPairs.second = value;
}

void sim_mob::xml::UniNode_t_pimpl::Connectors (std::set<std::pair<unsigned long,unsigned long> > value)
{
	//Save for later.
	connectors = value;
}


