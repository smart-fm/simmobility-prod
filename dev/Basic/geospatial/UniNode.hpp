/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <map>
#include <set>

#include "../constants.h"

#include "Node.hpp"

namespace sim_mob
{


//Forward declarations
class Point2D;
class Link;
class Lane;
class LaneConnector;
class RoadSegment;

namespace aimsun
{
//Forward declarations
class Loader;
}


/**
 * A Node where two to four RoadSegments from within the same Link meet. This usually occurs
 *   because lane rules (or the actual number of lanes) change.
 *
 * \note
 * Currently, UniNodes are slightly restricted: They MUST always contain either ONE or TWO
 *   paths through them, one in the forward direction and another in the reverse. See the note
 *   in Loader::ProcessSection() about this limitation. In the general case, we can say that a
 *   UniNode should simply be a Node that occurs between RoadSegments in the same Link.
 *
 * Each lane from an incoming segment connects directly to one Lane in an outgoing segment which
 *   is NOTE heading back to the same source node.
 */
class UniNode : public sim_mob::Node {
public:
	///Retrieve the outgoing Lane at this Node.
	const sim_mob::Lane* getOutgoingLane(const sim_mob::Lane& from) const;

	///Retrieve a pair of Road Segments representing the two Segments which meet at this node.
	///NOTE: We may choose to turn the pair into a class (see wiki notes on this), or we may
	///      simply separate this out into a "getRoadSegmentFrom", "getRoadSegmentTo"
	///      ---although, be careful with the second approach, since the "from" segment actually
	///      goes "to" the node.
	//std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> getRoadSegments() const;


	///Helper method: Build the connectors vector dynamically by aligning a lane in the "from" Road Segment with one
	/// in the "to" Road Segment.
	///NOTE: The "from/to" laneID pairs will definitely be cleaned up later; for now I'm just trying
	//       to get them to output something decent. At the moment they MUST correspond to "firstPair", "secondPair". ~Seth
	static void buildConnectorsFromAlignedLanes(UniNode* node, std::pair<unsigned int, unsigned int> fromToLaneIDs1, std::pair<unsigned int, unsigned int> fromToLaneIDs2);

	std::vector<const sim_mob::RoadSegment*> getRoadSegments() const;

protected:
	std::map<const sim_mob::Lane*, sim_mob::Lane* > connectors;

	///Bookkeeping: which RoadSegments meet at this Node?
	//  NOTE: If the RoadSegments in secondPair are null; then this is a one-way UniNode.
	//  As "from->to"
	std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> firstPair;
	std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> secondPair;


friend class sim_mob::aimsun::Loader;

};



}
