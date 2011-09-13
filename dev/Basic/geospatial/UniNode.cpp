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



vector<const RoadSegment*> sim_mob::UniNode::getRoadSegments() const
{
	//A little wordy, but it works.
	vector<const RoadSegment*> res;
	if (firstPair.first) {
		res.push_back(firstPair.first);
	}
	if (firstPair.second) {
		res.push_back(firstPair.second);
	}
	if (secondPair.first) {
		res.push_back(secondPair.first);
	}
	if (secondPair.second) {
		res.push_back(secondPair.second);
	}

	return res;
}



void sim_mob::UniNode::buildConnectorsFromAlignedLanes(UniNode* node, pair<unsigned int, unsigned int> fromToLaneIDs1, pair<unsigned int, unsigned int> fromToLaneIDs2)
{
	node->connectors.clear();

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

		//Get the "to" lane offset.
		int toOffset = static_cast<int>(fromToPair.second) - fromToPair.first;

		//Line up each lane. Handles merges.
		for (size_t fromID=0; fromID<segPair.first->getLanes().size(); fromID++) {
			//Convert the lane ID, but bound it to "to"'s actual number of available lanes.
			int toID = fromID + toOffset;
			toID = min<int>(max<int>(toID, 0), segPair.second->getLanes().size());

			//Link the two
			node->connectors[segPair.first->getLanes()[fromID]] = segPair.second->getLanes()[toID];
		}

		//Check for and handle branches.
		for (int i=0; i<toOffset; i++) {
			node->connectors[segPair.first->getLanes()[0]] = segPair.second->getLanes()[i];
		}
		size_t numFrom = segPair.first->getLanes().size();
		for (int i=numFrom+toOffset; i<(int)segPair.second->getLanes().size(); i++) {
			node->connectors[segPair.first->getLanes()[numFrom]] = segPair.second->getLanes()[i];
		}
	}
}


