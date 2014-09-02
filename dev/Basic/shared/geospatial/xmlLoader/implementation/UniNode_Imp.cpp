//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


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

	//Save the lane connectors for later.
	book.addUniNodeLaneConnectorCache(res, connectors);

	book.addUniNodeSegmentPairCache(res, segmentPairs);


	//NOTE: This retrieves the parent Node*, but it also allocates it. Replace it as a value type return if possible.
	sim_mob::Node tempNode = Node_t_pimpl::post_Node_t();
	res->location = sim_mob::Point2D(tempNode.getLocation());
	res->setID(tempNode.getID());
	res->originalDB_ID = tempNode.originalDB_ID;

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


