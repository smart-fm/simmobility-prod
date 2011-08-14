/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "UniNode.hpp"

#include "Lane.hpp"

using namespace sim_mob;

using std::pair;
using std::vector;
using std::max;
using std::min;


const Lane* sim_mob::UniNode::getOutgoingLane(const Lane& from) const
{
	if (connectors.count(&from)>0) {
		return connectors.find(&from)->second;
	}
	return nullptr;
}



pair<const RoadSegment*, const RoadSegment*> sim_mob::UniNode::getRoadSegments() const
{
	return pair<const RoadSegment*, const RoadSegment*>(segmentFrom, segmentTo);
}



void sim_mob::UniNode::buildConnectorsFromAlignedLanes(UniNode* node, unsigned int fromLaneID, unsigned int toLaneID)
{
	node->connectors.clear();

	//Get the "to" lane offset.
	int toOffset = static_cast<int>(toLaneID) - fromLaneID;

	//Line up each lane. Handles merges.
	for (size_t fromID=0; fromID<node->segmentFrom->getLanes().size(); fromID++) {
		//Convert the lane ID, but bound it to "to"'s actual number of available lanes.
		int toID = fromID + toOffset;
		toID = min<int>(max<int>(toID, 0), node->segmentTo->getLanes().size());

		//Link the two
		node->connectors[node->segmentFrom->getLanes()[fromID]] = node->segmentTo->getLanes()[toID];
	}

	//Check for and handle branches.
	for (int i=0; i<toOffset; i++) {
		node->connectors[node->segmentFrom->getLanes()[0]] = node->segmentTo->getLanes()[i];
	}
	size_t numFrom = node->segmentFrom->getLanes().size();
	for (int i=numFrom+toOffset; i<(int)node->segmentTo->getLanes().size(); i++) {
		node->connectors[node->segmentFrom->getLanes()[numFrom]] = node->segmentTo->getLanes()[i];
	}
}


