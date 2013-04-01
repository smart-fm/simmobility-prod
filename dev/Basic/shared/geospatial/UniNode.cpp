/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "UniNode.hpp"

#include <boost/thread.hpp>
#include "Lane.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "util/OutputUtil.hpp"

using namespace sim_mob;

using std::pair;
using std::vector;
using std::map;
using std::max;
using std::min;


const Lane* sim_mob::UniNode::getOutgoingLane(const Lane& from) const
{
	if (connectors.count(&from)>0) {
		return connectors.find(&from)->second;
	}
	return nullptr;
}
std::vector<sim_mob::Lane*> sim_mob::UniNode::getOutgoingLanes(Lane& from)
{
	std::vector<sim_mob::Lane*> res;
	std::map<const sim_mob::Lane*, sim_mob::Lane* >::iterator it;
	for (it = connectors.begin();it!=connectors.end();++it)
	{
		const sim_mob::Lane * l = it->first;
		if(l->getLaneID() == from.getLaneID())
		{
			res.push_back(it->second);
		}
	}
	return res;
}

const std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*>& sim_mob::UniNode::getRoadSegmentPair(bool first) const
{
	if(first) return firstPair;
	return secondPair;
}

const vector<const RoadSegment*>& sim_mob::UniNode::getRoadSegments() const
{
	//A little wordy, but it works.
	if (cachedSegmentsList.empty()) {
		if (firstPair.first) {
			cachedSegmentsList.push_back(firstPair.first);
		}
		if (firstPair.second) {
			cachedSegmentsList.push_back(firstPair.second);
		}
		if (secondPair.first) {
			cachedSegmentsList.push_back(secondPair.first);
		}
		if (secondPair.second) {
			cachedSegmentsList.push_back(secondPair.second);
		}
	}

	if (cachedSegmentsList.empty()) {
		Warn() <<"Warning: UniNode segment list still empty after call to \"set\"" <<std::endl;
	}

	return cachedSegmentsList;
}



UniNode::UniLaneConnector sim_mob::UniNode::getForwardLanes(const Lane& from) const
{
	map<const Lane*, UniLaneConnector>::const_iterator it = new_connectors.find(&from);
	if (it==new_connectors.end()) {
		return UniLaneConnector();
	}
	return it->second;;
}

const Lane* sim_mob::UniNode::getForwardDrivingLane(const sim_mob::Lane& from) const
{
	//Simple case
	UniLaneConnector lc = getForwardLanes(from);
	if (lc.center) {
		return lc.center;
	}

	//We don't want to make arbitrary decisions (let the agent do that), so only return left/right if the other is null.
	if (lc.left && !lc.right) {
		return lc.left;
	} else if (lc.right && !lc.left) {
		return lc.right;
	}
	return nullptr;
}



void sim_mob::UniNode::buildConnectorsFromAlignedLanes(UniNode* node, pair<unsigned int, unsigned int> fromToLaneIDs1, pair<unsigned int, unsigned int> fromToLaneIDs2)
{
	node->cachedSegmentsList.clear();
	node->connectors.clear();
	node->new_connectors.clear();

	//Compute for each pair of Segments at this node
	for (size_t runID=0; runID<2; runID++) {
		//Reference
		pair<const RoadSegment*, const RoadSegment*>& segPair = (runID==0)?node->firstPair:node->secondPair;
		pair<unsigned int, unsigned int>& fromToPair = (runID==0)?fromToLaneIDs1:fromToLaneIDs2;

		//No data?
		if (!segPair.first || !segPair.second) {
			if (runID==1) {
				break; //One-way segment
			}
			throw std::runtime_error("Attempting to build connectors on UniNode with no primary path.");
		}

		//Dispatch
		buildConnectorsFromAlignedLanes(node, segPair.first, segPair.second, fromToPair.first, fromToPair.second);
		buildNewConnectorsFromAlignedLanes(node, segPair.first, segPair.second, fromToPair.first, fromToPair.second);
	}
}


void sim_mob::UniNode::buildNewConnectorsFromAlignedLanes(UniNode* node, const RoadSegment* fromSeg, const RoadSegment* toSeg, unsigned int fromAlignLane, unsigned int toAlignLane)
{
	//Get the "to" lane offset.
	int alignOffset = static_cast<int>(toAlignLane) - static_cast<int>(fromAlignLane);

	//We build a lookup based on the "from" lanes.
	for (size_t fromID=0; fromID<fromSeg->getLanes().size(); fromID++) {
		//We consider "left", "center", and "right", based on the original offset lane.
		int toIDCenter = static_cast<int>(fromID) + alignOffset;
		UniLaneConnector lc;
		lc.left = toSeg->getLane(toIDCenter-1);
		lc.center = toSeg->getLane(toIDCenter);
		lc.right = toSeg->getLane(toIDCenter+1);

		//Save it.
		node->new_connectors[fromSeg->getLane(fromID)] = lc;
	}

}


void sim_mob::UniNode::buildConnectorsFromAlignedLanes(UniNode* node, const RoadSegment* fromSeg, const RoadSegment* toSeg, unsigned int fromAlignLane, unsigned int toAlignLane)
{
	//Get the "to" lane offset.
	int toOffset = static_cast<int>(toAlignLane) - fromAlignLane;

	//Line up each lane. Handles merges.
	for (size_t fromID=0; fromID<fromSeg->getLanes().size(); fromID++) {
		//Convert the lane ID, but bound it to "to"'s actual number of available lanes.
		int toID = fromID + toOffset;
		toID = min<int>(max<int>(toID, 0), toSeg->getLanes().size()-1);

		//Link the two
		Lane* from = fromSeg->getLanes()[fromID];
		Lane* to = toSeg->getLanes()[toID];
		node->connectors[from] = to;
	}

	//Check for and handle branches.
	for (int i=0; i<toOffset; i++) {
		node->connectors[fromSeg->getLanes()[0]] = toSeg->getLanes()[i];
	}
	size_t numFrom = fromSeg->getLanes().size()-1;
	for (int i=numFrom+toOffset; i<(int)toSeg->getLanes().size(); i++) {
		node->connectors[fromSeg->getLanes()[numFrom]] = toSeg->getLanes()[i];
	}
}


