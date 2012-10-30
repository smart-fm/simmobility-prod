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
void ProcessMultiNodeConnectors(std::vector<sim_mob::MultiNode*>& nodes) {
	for(std::vector<sim_mob::MultiNode*>::iterator it = nodes.begin(); it!=nodes.end(); it++) {
		ProcessMultiNodeConnectors(*it, intersection_t_pimpl::GetConnectors(*it));
	}
}

//Helper: Handle "RoadSegmentsAt" for UniNodes
void ProcessUniNodeSegments(std::set<sim_mob::UniNode*>& nodes) {
	for(std::set<sim_mob::UniNode*>::iterator nodeIt = nodes.begin(); nodeIt!=nodes.end(); nodeIt++) {
		(*nodeIt)->firstPair.first   = Segments_pimpl::LookupSegment(UniNode_t_pimpl::GetSegmentPairs(*nodeIt).first.first);
		(*nodeIt)->firstPair.second  = Segments_pimpl::LookupSegment(UniNode_t_pimpl::GetSegmentPairs(*nodeIt).first.second);
		(*nodeIt)->secondPair.first  = Segments_pimpl::LookupSegment(UniNode_t_pimpl::GetSegmentPairs(*nodeIt).second.first);
		(*nodeIt)->secondPair.second = Segments_pimpl::LookupSegment(UniNode_t_pimpl::GetSegmentPairs(*nodeIt).second.second);	  }
}

//Helper: Handle "RoadSegmentsAt" for MultiNodes
void ProcessMultiNodeSegments(std::vector<sim_mob::MultiNode*>& nodes) {
	for(std::vector<sim_mob::MultiNode*>::iterator nodeIt = nodes.begin(); nodeIt!=nodes.end(); nodeIt++) {
		intersection_t_pimpl::RoadSegmentSet segIDs = intersection_t_pimpl::GetSegmentsAt(*nodeIt);
		for (std::set<unsigned long>::iterator segIdIt=segIDs.begin(); segIdIt!=segIDs.end(); segIdIt++) {
			(*nodeIt)->addRoadSegmentAt(Segments_pimpl::LookupSegment(*segIdIt));
		}
	}
}

} //End unnamed namespace


void sim_mob::xml::GeoSpatial_t_pimpl::pre ()
{
}

void sim_mob::xml::GeoSpatial_t_pimpl::post_GeoSpatial_t ()
{
}


void sim_mob::xml::GeoSpatial_t_pimpl::RoadNetwork ()
{
	//TODO: Retrieving the RoadNetwork statically is a bad idea.
	sim_mob::RoadNetwork& rn = ConfigParams::GetInstance().getNetworkRW();

	//Process various left-over items.
	ProcessUniNodeConnectors(rn.getUniNodesRW());
	ProcessUniNodeSegments(rn.getUniNodesRW());
	ProcessMultiNodeConnectors(rn.getNodesRW());
	ProcessMultiNodeSegments(rn.getNodesRW());

	//Process LinkLocs
	std::map<sim_mob::Node*, unsigned int>& linkLocs = Node_t_pimpl::GetLinkLocList();
	for (std::map<sim_mob::Node*, unsigned int>::iterator it=linkLocs.begin(); it!=linkLocs.end(); it++) {
		it->first->setLinkLoc(Links_pimpl::LookupLink(it->second));
	}
}

