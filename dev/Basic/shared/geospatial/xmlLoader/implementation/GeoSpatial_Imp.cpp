#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


namespace {

//Helper: Create LaneConnector entries for a single UniNode
void ProcessUniNodeConnectors(sim_mob::UniNode* node, UniNode_t_pimpl::LaneConnectSet connectors) {
	for(UniNode_t_pimpl::LaneConnectSet::iterator it = connectors.begin(); it!=connectors.end(); it ++) {
		node->setConnectorAt(Lanes_pimpl::LookupLane(it->first), Lanes_pimpl::LookupLane(it->second));
	}
}

//Helper: Create LaneConnector entries for all UniNodes
void ProcessUniNodeConnectors(std::set<sim_mob::UniNode*>& nodes) {
	for(std::set<sim_mob::UniNode*>::iterator it = nodes.begin(); it!=nodes.end(); it ++) {
		ProcessUniNodeConnectors(*it, UniNode_t_pimpl::GetConnectors(*it));
	}
}


//Helper: Create LaneConnector entries for a single MultiNode
void ProcessMultiNodeConnectors(sim_mob::MultiNode* node, intersection_t_pimpl::LaneConnectSet connectors) {
	for(intersection_t_pimpl::LaneConnectSet::iterator it = connectors.begin(); it!=connectors.end(); it ++) {
		std::set<sim_mob::LaneConnector*> connectors;
		helper::UniNodeConnectors& rawConnectors = it->second; //reminder: We don't have any uninode here. it is just a name paw. we are re-Using :)
		for(helper::UniNodeConnectors::iterator laneIt = rawConnectors.begin(); laneIt != rawConnectors.end(); laneIt++) {
			sim_mob::LaneConnector* lc = new sim_mob::LaneConnector(Lanes_pimpl::LookupLane(laneIt->first), Lanes_pimpl::LookupLane(laneIt->second));
			connectors.insert(lc);
		}

		//Save it to the RoadSegment
		sim_mob::RoadSegment* rs = Segments_pimpl::LookupSegment(it->first);
		node->setConnectorAt(rs, connectors);
	}
}

//Helper: Create LaneConnector entries for all MultiNodes
void ProcessMultiNodeConnectors(const helper::Bookkeeping& book, std::vector<sim_mob::MultiNode*>& nodes) {
	for(std::vector<sim_mob::MultiNode*>::iterator it = nodes.begin(); it!=nodes.end(); it++) {
		ProcessMultiNodeConnectors(*it, book.getMultiNodeLaneConnectorCache(*it));
	}
}

//Helper: Handle "RoadSegmentsAt" for MultiNodes (Node)
void CacheRoadSegmentsAtMultiNodes(sim_mob::Node* node, sim_mob::RoadSegment* rs) {
	sim_mob::MultiNode* mn = dynamic_cast<sim_mob::MultiNode*>(node);
	if (mn) {
		//We can take advantage of the fact that this is a set and just insert it; guaranteed no duplicates.
		mn->addRoadSegmentAt(rs);
	}
}

//Helper: Handle "RoadSegmentsAt" for MultiNodes (Segment->Node)
void CacheRoadSegmentsAtMultiNodes(std::vector<sim_mob::RoadSegment*>& roadway) {
	for (std::vector<sim_mob::RoadSegment*>::iterator it=roadway.begin(); it!=roadway.end(); it++) {
		//NOTE: Just add to both start and end; make no assumptions that roads start/end one after the other.
		CacheRoadSegmentsAtMultiNodes((*it)->getStart(), *it);
		CacheRoadSegmentsAtMultiNodes((*it)->getEnd(), *it);
	}
}

//Helper: Handle "RoadSegmentsAt" for MultiNodes (Link->Segment)
void CacheRoadSegmentsAtMultiNodes(std::vector<sim_mob::Link*>& links) {
	for (std::vector<sim_mob::Link*>::iterator it=links.begin(); it!=links.end(); it++) {
		CacheRoadSegmentsAtMultiNodes((*it)->getPath(true));
		CacheRoadSegmentsAtMultiNodes((*it)->getPath(false));
	}
}

//Helper: Handle "RoadSegmentsAt" for UniNodes
void CacheRoadSegmentsAtUniNodes(std::set<sim_mob::UniNode*>& nodes) {
	for (std::set<sim_mob::UniNode*>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		//We can take advantage of the fact that UniNodes will calculate this themselves.
		(*it)->getRoadSegments();
	}
}

} //End unnamed namespace


void sim_mob::xml::GeoSpatial_t_pimpl::pre ()
{
}

void sim_mob::xml::GeoSpatial_t_pimpl::post_GeoSpatial_t ()
{
}


void sim_mob::xml::GeoSpatial_t_pimpl::RoadNetwork (sim_mob::RoadNetwork& rn)
{
	//TODO: Retrieving the RoadNetwork statically is a bad idea.
	//sim_mob::RoadNetwork& rn = ConfigParams::GetInstance().getNetworkRW();

	//Parse and save "RoadSegmentsAt"
	CacheRoadSegmentsAtUniNodes(rn.getUniNodes());
	CacheRoadSegmentsAtMultiNodes(rn.getLinks());


	//Process various left-over items.
	ProcessUniNodeConnectors(rn.getUniNodes());
	ProcessMultiNodeConnectors(book, rn.getNodes());

	//Process LinkLocs
	std::map<sim_mob::Node*, unsigned int>& linkLocs = Node_t_pimpl::GetLinkLocList();
	for (std::map<sim_mob::Node*, unsigned int>::iterator it=linkLocs.begin(); it!=linkLocs.end(); it++) {
		it->first->setLinkLoc(Links_pimpl::LookupLink(it->second));
	}
}

