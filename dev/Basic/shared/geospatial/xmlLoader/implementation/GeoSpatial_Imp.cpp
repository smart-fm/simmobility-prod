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


//Helper: Create LaneConnector entries for a single UniNode
void ProcessMultiNodeConnectors(sim_mob::MultiNode* node, intersection_t_pimpl::LaneConnectSet connectors) {
	for(intersection_t_pimpl::LaneConnectSet::iterator it = connectors.begin(); it!=connectors.end(); it ++) {
		std::set<sim_mob::LaneConnector*> connectors;
		helper::UniNodeConnectors& rawConnectors = it->second; //reminder: We don't have any uninode here. it is just a name paw. we are re-Using :)
		for(helper::UniNodeConnectors::iterator it = rawConnectors.begin(); it != rawConnectors.end(); it++) {
			sim_mob::LaneConnector* lc = new sim_mob::LaneConnector(Lanes_pimpl::LookupLane(it->first), Lanes_pimpl::LookupLane(it->second));
			connectors.insert(lc);
		}

		//Save it to the RoadSegment
		sim_mob::RoadSegment* rs = Segments_pimpl::LookupSegment(it->first);
		node->setConnectorAt(rs, connectors);
	}
}

//Helper: Create LaneConnector entries for all UniNodes
void ProcessMultiNodeConnectors(std::set<sim_mob::MultiNode*>& nodes) {
	for(std::set<sim_mob::MultiNode*>::iterator it = nodes.begin(); it!=nodes.end(); it ++) {
		ProcessMultiNodeConnectors(*it, intersection_t_pimpl::GetConnectors(*it));
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
	ProcessMultiNodeConnectors(rn.getNodesRW());



	  std::cout << "In GeoSpatial_t_pimpl.RoadNetwork ()\n";
//	  Now Take care of items like lane 'connectors' , 'road segments at' multinodes etc
	  //multinodes RoadSegmentAt
	  std::vector<sim_mob::MultiNode*>& mNodes = ConfigParams::GetInstance().getNetworkRW().getNodesRW();
	  for(std::vector<sim_mob::MultiNode*>::iterator node_it = mNodes.begin(); node_it != mNodes.end(); node_it ++)
	  {
		  for(std::set<unsigned long>::iterator rs_it = geo_RoadSegmentsAt[(*node_it)->getID()].begin(), it_end(geo_RoadSegmentsAt[(*node_it)->getID()].end()); rs_it != it_end ; rs_it++)
		  {
			  (*node_it)->addRoadSegmentAt(geo_Segments_[*rs_it]);
		  }
	  }

	  //multi node connectors

//	  //will not be needed in the new version of road network graphs
//	  //multinodes roadSegmentsCircular
//	  sim_mob::RoadNetwork& rn = ConfigParams::GetInstance().getNetworkRW();
//	  for(std::vector<sim_mob::MultiNode*>::iterator node_it = mNodes.begin(); node_it != mNodes.end(); node_it ++)
//	  {
//		  sim_mob::MultiNode::BuildClockwiseLinks(rn, *node_it);
//	  }

	  //uni nodes
	  //uninode segmentpairs
	  //uni node connectors
	  //uninode segment pairs
	  //you are welcom to integrate this loop to the above loop, I just separated them for simplicity
	  for(std::set<sim_mob::UniNode*>::iterator node_it = uNodes.begin(); node_it != uNodes.end(); node_it ++)
	  {
//		  declared variable to enforce constantness here instead of const_cast later
		  const sim_mob::RoadSegment* firstPair_first = geo_Segments_[geo_UniNode_SegmentPairs[(*node_it)->getID()].first.first];
		  const sim_mob::RoadSegment* firstPair_second = geo_Segments_[geo_UniNode_SegmentPairs[(*node_it)->getID()].first.second];
		  const sim_mob::RoadSegment* secondPair_first = geo_Segments_[geo_UniNode_SegmentPairs[(*node_it)->getID()].second.first];
		  const sim_mob::RoadSegment* secondPair_second = geo_Segments_[geo_UniNode_SegmentPairs[(*node_it)->getID()].second.second];
		  (*node_it)->firstPair = std::make_pair(firstPair_first,firstPair_second);
		  (*node_it)->secondPair = std::make_pair(secondPair_first,secondPair_second);
	  }
//	  //linkLoc //todo later
	  geo_LinkLoc_random & linkLocs = get<0>(geo_LinkLoc_);
	  for(geo_LinkLoc_random::iterator link_it = linkLocs.begin(), it_end(linkLocs.end()); link_it != it_end; link_it++)
	  {
		  for(std::vector<sim_mob::Node*>::iterator node_it = link_it->node.begin(); node_it != link_it->node.end() ; node_it++)
		  {
			 (*node_it)->setLinkLoc( geo_Links_[link_it->linkID]);
		  }
	  }
}

