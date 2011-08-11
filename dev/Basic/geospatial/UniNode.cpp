/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "UniNode.hpp"

#include "Lane.hpp"

using namespace sim_mob;

using std::pair;
using std::vector;
using std::max;
using std::min;


sim_mob::UniNode::UniNode(const RoadSegment* from, const RoadSegment* to) : segmentFrom(from), segmentTo(to)
{
}


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



void sim_mob::UniNode::buildConnectorsFromAlignedLanes(unsigned int fromLaneID, unsigned int toLaneID)
{
	connectors.clear();

	//Get the "to" lane offset.
	int toOffset = static_cast<int>(toLaneID) - fromLaneID;

	//Line up each lane. Handles merges.
	for (size_t fromID=0; fromID<segmentFrom->getLanes().size(); fromID++) {
		//Convert the lane ID, but bound it to "to"'s actual number of available lanes.
		int toID = fromID + toOffset;
		toID = min<int>(max<int>(toID, 0), segmentTo->getLanes().size());

		//Link the two
		connectors[segmentFrom->getLanes()[fromID]] = segmentTo->getLanes()[toID];
	}

	//Check for and handle branches.
	for (int i=0; i<toOffset; i++) {
		connectors[segmentFrom->getLanes()[0]] = segmentTo->getLanes()[i];
	}
	size_t numFrom = segmentFrom->getLanes().size();
	for (int i=numFrom+toOffset; i<(int)segmentTo->getLanes().size(); i++) {
		connectors[segmentFrom->getLanes()[numFrom]] = segmentTo->getLanes()[i];
	}
}


