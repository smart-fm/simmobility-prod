#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


//TODO: Using a static field like this means we can't support multi-threaded loading of different networks.
//      It shouldn't be too hard to add some "temporary" object at a scope global to the parser classes. ~Seth
/*std::map<sim_mob::MultiNode*, intersection_t_pimpl::LaneConnectSet> sim_mob::xml::intersection_t_pimpl::ConnectCache;
std::map<sim_mob::MultiNode*, intersection_t_pimpl::RoadSegmentSet> sim_mob::xml::intersection_t_pimpl::SegmentsAtCache;

//Register a set of connectors for retrieval later.
void sim_mob::xml::intersection_t_pimpl::RegisterConnectors(sim_mob::MultiNode* intersection, const intersection_t_pimpl::LaneConnectSet& connectors)
{
	if (ConnectCache.count(intersection)>0) {
		throw std::runtime_error("Intersection connectors are already registered.");
	}
	ConnectCache[intersection] = connectors;
}


intersection_t_pimpl::LaneConnectSet sim_mob::xml::intersection_t_pimpl::GetConnectors(sim_mob::MultiNode* intersection)
{
	std::map<sim_mob::MultiNode*, intersection_t_pimpl::LaneConnectSet>::iterator it = ConnectCache.find(intersection);
	if (it!=ConnectCache.end()) {
		return it->second;
	}
	return intersection_t_pimpl::LaneConnectSet(); //Just return an empty set; there may be no connectors.
}

//Register a set of segments for retrieval later.
void sim_mob::xml::intersection_t_pimpl::RegisterSegmentsAt(sim_mob::MultiNode* intersection, const intersection_t_pimpl::RoadSegmentSet& segmentsAt)
{
	if (SegmentsAtCache.count(intersection)>0) {
		throw std::runtime_error("RoadSegments connectors are already registered.");
	}
	SegmentsAtCache[intersection] = segmentsAt;
}


intersection_t_pimpl::RoadSegmentSet sim_mob::xml::intersection_t_pimpl::GetSegmentsAt(sim_mob::MultiNode* intersection)
{
	std::map<sim_mob::MultiNode*, intersection_t_pimpl::RoadSegmentSet>::iterator it = SegmentsAtCache.find(intersection);
	if (it!=SegmentsAtCache.end()) {
		return it->second;
	}
	return intersection_t_pimpl::RoadSegmentSet(); //Just return an empty set; there may be no segments here.
}*/


void sim_mob::xml::intersection_t_pimpl::pre ()
{
	//Chain to parent.
	Node_t_pimpl::pre();
	model = sim_mob::Intersection(0, 0);
	segmentsAt.clear();
	connectors.clear();
}

sim_mob::MultiNode* sim_mob::xml::intersection_t_pimpl::post_intersection_t ()
{
	//NOTE: This retrieves the parent class's value, but it also allocates memory.
	//sim_mob::Node* v (post_Node_t ());

	sim_mob::Intersection* res = new sim_mob::Intersection(model);

	//Save the set of connectors/segments for later, since we can't construct it until Lanes have been loaded.
	//intersection_t_pimpl::RegisterConnectors(res, connectors);
	//intersection_t_pimpl::RegisterSegmentsAt(res, segmentsAt);

	//Copy over additional properties.
	sim_mob::Node tempNode = Node_t_pimpl::post_Node_t();
	res->location = sim_mob::Point2D(tempNode.getLocation());
	res->setID(tempNode.getID());
	res->originalDB_ID = tempNode.originalDB_ID;
	//RegisterLinkLoc(res, linkLocSaved);

	//Nodes_pimpl::RegisterNode(res->getID(), res);

	return res;
}

void sim_mob::xml::intersection_t_pimpl::roadSegmentsAt (std::set<unsigned long> value)
{
	segmentsAt = value;
}

void sim_mob::xml::intersection_t_pimpl::Connectors (const LaneConnectSet& value)
{
	connectors = value;
}

void sim_mob::xml::intersection_t_pimpl::ChunkLengths ()
{
}

void sim_mob::xml::intersection_t_pimpl::Offsets ()
{
}

void sim_mob::xml::intersection_t_pimpl::Separators ()
{
}

void sim_mob::xml::intersection_t_pimpl::additionalDominantLanes ()
{
}

void sim_mob::xml::intersection_t_pimpl::additionalSubdominantLanes ()
{
}

void sim_mob::xml::intersection_t_pimpl::domainIslands ()
{
}


