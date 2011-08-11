/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Intersection.hpp"

#include "LaneConnector.hpp"
#include "RoadSegment.hpp"

using std::vector;
using std::map;
using std::set;
using namespace sim_mob;


bool sim_mob::Intersection::hasOutgoingLanes(const RoadSegment& from) const
{
	return connectors.count(&from) > 0;
}


const set<LaneConnector*>& sim_mob::Intersection::getOutgoingLanes(const RoadSegment& from) const
{
	if (!hasOutgoingLanes(from)) {
		//TODO: How are we handling logical errors?
		throw std::runtime_error("No outgoing Road Segments.");
	}

	return connectors.find(&from)->second;
}

