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
 * A Node where exactly two RoadSegments from within the same Link meet. This usually represents
 * a change in the number of lanes (or sometimes just the lane rules that are in effect).
 * Each lane from the first segment connects directly to one Lane in the second segment.
 */
class SegmentNode : public sim_mob::Node {
public:
	///Construct this SegmentNode with the given Road Segments
	SegmentNode(const sim_mob::RoadSegment* from=nullptr, const sim_mob::RoadSegment* to=nullptr);

	///Retrieve the outgoing Lane at this Node.
	const sim_mob::Lane* getOutgoingLane(const sim_mob::Lane& from) const;

	///Retrieve a pair of Road Segments representing the two Segments which meet at this node.
	///NOTE: We may choose to turn the pair into a class (see wiki notes on this), or we may
	///      simply separate this out into a "getRoadSegmentFrom", "getRoadSegmentTo"
	///      ---although, be careful with the second approach, since the "from" segment actually
	///      goes "to" the node.
	std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> getRoadSegments() const;

	///Helper method: Build the connectors vector dynamically by aligning a lane in the "from" Road Segment with one
	/// in the "to" Road Segment.
	void buildConnectorsFromAlignedLanes(unsigned int fromLaneID, unsigned int toLaneID);

protected:
	std::map<const sim_mob::Lane*, sim_mob::Lane* > connectors;

	///Bookkeeping: which RoadSegments meet at this Node?
	const sim_mob::RoadSegment* segmentFrom;
	const sim_mob::RoadSegment* segmentTo;


friend class sim_mob::aimsun::Loader;

};





}
