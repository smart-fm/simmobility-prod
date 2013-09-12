//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


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
	book.addMultiNodeLaneConnectorCache(res, connectors);

	//Copy over additional properties.
	sim_mob::Node tempNode = Node_t_pimpl::post_Node_t();
	res->location = sim_mob::Point2D(tempNode.getLocation());
	res->setID(tempNode.getID());
	res->originalDB_ID = tempNode.originalDB_ID;
	//RegisterLinkLoc(res, linkLocSaved);

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


