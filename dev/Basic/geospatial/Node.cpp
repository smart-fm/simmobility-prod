/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Node.hpp"
#include "LaneConnector.hpp"
#include "RoadSegment.hpp"

using std::vector;
using std::map;
using std::set;
using namespace sim_mob;



/*void sim_mob::Node::addLaneConnector(Lane* from, Lane* to)
{
	//Ensure both RoadSegments are represented
	roadSegmentsAt.insert(from->getRoadSegment());
	roadSegmentsAt.insert(to->getRoadSegment());

	//Add it. (vector should auto-initialize)
	connectors[from].insert(to);
}*/



/*vector<RoadSegment*> sim_mob::Node::getAllRoadSegments(bool fromHere, bool toHere) const
{
	//Build it up
	vector<Lane*> res;
	for (map<Lane*, set<Lane*> >::iterator it=connectors.begin(); it!=connectors.end(); it++) {
		if (toHere) {
			res.push_back(it->first);
		}
		if (fromHere) {
			res.insert(res.end(), it->second.begin(), it->second.end());
		}
	}

	return res;
}*/


bool sim_mob::Node::hasOutgoingLanes(const RoadSegment& from) const
{
	return connectors.count(&from) > 0;
}


const set<LaneConnector*>& sim_mob::Node::getOutgoingLanes(const RoadSegment& from) const
{
	if (!hasOutgoingLanes(from)) {
		//TODO: How are we handling logical errors?
		throw std::runtime_error("No outgoing Road Segments.");
	}

	return connectors.find(&from)->second;
}


