#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


namespace {

//Helper: Create LaneConnector entries for a single UniNode
void ProcessUniNodeConnectors(const helper::Bookkeeping& book, sim_mob::UniNode* node, helper::Bookkeeping::UNConnect connectors) {
	for(helper::Bookkeeping::UNConnect::iterator it = connectors.begin(); it!=connectors.end(); it ++) {
		node->setConnectorAt(book.getLane(it->first), book.getLane(it->second));
	}
}

//Helper: Create LaneConnector entries for all UniNodes
void ProcessUniNodeConnectors(const helper::Bookkeeping& book,std::set<sim_mob::UniNode*>& nodes) {
	for(std::set<sim_mob::UniNode*>::iterator it = nodes.begin(); it!=nodes.end(); it ++) {
		ProcessUniNodeConnectors(book, *it, book.getUniNodeLaneConnectorCache(*it));
	}
}


//Helper: Create LaneConnector entries for a single MultiNode
void ProcessMultiNodeConnectors(const helper::Bookkeeping& book,sim_mob::MultiNode* node, helper::Bookkeeping::MNConnect connectors) {
	for(helper::Bookkeeping::MNConnect::iterator it = connectors.begin(); it!=connectors.end(); it ++) {
		std::set<sim_mob::LaneConnector*> connectors;
		helper::UniNodeConnectors& rawConnectors = it->second; //reminder: We don't have any uninode here. it is just a name paw. we are re-Using :)
		for(helper::UniNodeConnectors::iterator laneIt = rawConnectors.begin(); laneIt != rawConnectors.end(); laneIt++) {
			sim_mob::LaneConnector* lc = new sim_mob::LaneConnector(book.getLane(laneIt->first), book.getLane(laneIt->second));
			connectors.insert(lc);
		}

		//Save it to the RoadSegment
		sim_mob::RoadSegment* rs = book.getSegment(it->first);
		node->setConnectorAt(rs, connectors);
	}
}

//Helper: Create LaneConnector entries for all MultiNodes
void ProcessMultiNodeConnectors(const helper::Bookkeeping& book, std::vector<sim_mob::MultiNode*>& nodes) {
	for(std::vector<sim_mob::MultiNode*>::iterator it = nodes.begin(); it!=nodes.end(); it++) {
		ProcessMultiNodeConnectors(book, *it, book.getMultiNodeLaneConnectorCache(*it));
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
		sim_mob::Node* start = (*it)->getStart();
		sim_mob::Node* end = (*it)->getEnd();
		CacheRoadSegmentsAtMultiNodes(start, *it);
		CacheRoadSegmentsAtMultiNodes(end, *it);
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
	//Parse and save "RoadSegmentsAt"
	CacheRoadSegmentsAtUniNodes(rn.getUniNodes());
	CacheRoadSegmentsAtMultiNodes(rn.getLinks());

	//Process various left-over items.
	ProcessUniNodeConnectors(book, rn.getUniNodes());
	ProcessMultiNodeConnectors(book, rn.getNodes());
}

