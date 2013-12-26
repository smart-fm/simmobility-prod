//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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

//Helper: Create LaneConnector entries for a single UniNode
void ProcessUniNodeNewConnectors(const helper::Bookkeeping& book, sim_mob::UniNode* node, helper::Bookkeeping::UNNConnect new_connectors) {
	helper::Bookkeeping::UNNConnect::iterator it = new_connectors.begin();
	if(new_connectors.begin()== new_connectors.end()){
		std::cout << "opps new_connectors is emptyfor uninode  " <<  node->getID() << std::endl;
	}
	for( ;it!= new_connectors.end(); it ++) {
//		std::cout << "Processing Tupple :" << it->first << " : " << it->second.get<0>() << " : " << it->second.get<1>() << " : " << it->second.get<2>()  << std::endl;
		boost::tuple<unsigned long,unsigned long,unsigned long> laneIds = it->second;
		boost::tuple<sim_mob::Lane*,sim_mob::Lane*,sim_mob::Lane*> lanePtrs;
		unsigned long id = boost::get<0>(it->second);
		if(id > 0){
			lanePtrs.get<0>() = book.getLane(boost::get<0>(it->second));
		}
		id = boost::get<1>(it->second);
		if(id > 0){
			lanePtrs.get<1>() = book.getLane(boost::get<1>(it->second));
		}
		id = boost::get<2>(it->second);
		if(id > 0){
			lanePtrs.get<2>() = book.getLane(boost::get<2>(it->second));
		}
		node->setNewConnectorAt(book.getLane(it->first), lanePtrs);

	}
}
//Helper: Create new LaneConnector entries for all UniNodes
void ProcessUniNodeNewConnectors(const helper::Bookkeeping& book,std::set<sim_mob::UniNode*>& nodes) {
	for(std::set<sim_mob::UniNode*>::iterator it = nodes.begin(); it!=nodes.end(); it ++) {
		ProcessUniNodeNewConnectors(book, *it, book.getUniNodeNewLaneConnectorCache(*it));
	}
}

//Helper: Create segment pairs entries for a single UniNode
void ProcessUniNodeSegPairs(const helper::Bookkeeping& book, sim_mob::UniNode* node, helper::Bookkeeping::SegPair segPair) {
	node->firstPair = std::make_pair(book.getSegment(segPair.first.first), book.getSegment(segPair.first.second));

	//Second pair may not exist
	if (segPair.second.first!=0 && segPair.second.second!=0) {
		node->secondPair = std::make_pair(book.getSegment(segPair.second.first), book.getSegment(segPair.second.second));
	}
}

//Helper: Save segment pairs from UniNodes
void ProcessUniNodeSegPairs(const helper::Bookkeeping& book,std::set<sim_mob::UniNode*>& nodes) {
	for(std::set<sim_mob::UniNode*>::iterator it = nodes.begin(); it!=nodes.end(); it ++) {
		ProcessUniNodeSegPairs(book, *it, book.getUniNodeSegmentPairCache(*it));
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
		CacheRoadSegmentsAtMultiNodes((*it)->getSegments());
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
	//This needs to be done first
	ProcessUniNodeSegPairs(book, rn.getUniNodes());

	//Parse and save "RoadSegmentsAt"
	CacheRoadSegmentsAtUniNodes(rn.getUniNodes());
	CacheRoadSegmentsAtMultiNodes(rn.getLinks());

	//Process various left-over items.
	ProcessUniNodeConnectors(book, rn.getUniNodes());
	ProcessUniNodeNewConnectors(book, rn.getUniNodes());
	ProcessMultiNodeConnectors(book, rn.getNodes());

	//Ugh; this shouldn't be needed.... (see Loader.cpp)
	//TODO: Remove this; Lanes should never have to be "generated" at runtime (or if so, the conditions should be fully controlled).
	{
	std::map<std::pair<sim_mob::Node*, sim_mob::Node*>, sim_mob::Link*> startEndLinkMap;
	//Scan first.
	for (std::vector<sim_mob::Link*>::iterator linkIt=rn.getLinks().begin(); linkIt!=rn.getLinks().end(); linkIt++) {
		startEndLinkMap[std::make_pair((*linkIt)->getStart(), (*linkIt)->getEnd())] = *linkIt;
	}
	//Now tag
	for (std::vector<sim_mob::Link*>::iterator linkIt=rn.getLinks().begin(); linkIt!=rn.getLinks().end(); linkIt++) {
		bool hasOpp = startEndLinkMap.count(std::make_pair((*linkIt)->getEnd(), (*linkIt)->getStart()))>0;
		(*linkIt)->hasOpposingLink = hasOpp ? 1 : 0;
	}
	}
}

