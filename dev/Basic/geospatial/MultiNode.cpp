/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "MultiNode.hpp"

#include "Lane.hpp"

using namespace sim_mob;

using std::pair;
using std::vector;
using std::set;


bool sim_mob::MultiNode::hasOutgoingLanes(const RoadSegment& from) const
{
	return connectors.count(&from) > 0;
}


const set<LaneConnector*>& sim_mob::MultiNode::getOutgoingLanes(const RoadSegment& from) const
{
	if (!hasOutgoingLanes(from)) {
		//TODO: How are we handling logical errors?
		throw std::runtime_error("No outgoing Road Segments.");
	}

	return connectors.find(&from)->second;
}

