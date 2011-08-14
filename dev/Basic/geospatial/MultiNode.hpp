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
 * A Node where any number of Links meet, usually representing an Interserction or Roundabout.
 * Each Link contributes one RoadSegment to this MultiNode, and each Lane of each RoadSegment
 * may connect to any number of lanes from any number of other RoadSegments.
 */
class MultiNode : public sim_mob::Node {
public:
	///Query the list of connectors at the current node, restricting the results to
	///   those which originate at the "from" segment.
	///Fails if no outgoing Lanes exist.
	///The reference to this vector may be invalidated when a new connector is added
	///   to this Node which links from a Lane* that is not currently linked.
	const std::set<sim_mob::LaneConnector*>& getOutgoingLanes(const sim_mob::RoadSegment& from) const;

	///Test if connetors exist at this node.
	bool hasOutgoingLanes(const sim_mob::RoadSegment& from) const;

	///Retrieve a list of all RoadSegments at this node.
	const std::set<sim_mob::RoadSegment*>& getRoadSegments() const { return roadSegmentsAt; }

protected:
	///Mapping from RoadSegment* -> set<LaneConnector*> representing lane connectors.
	///Currently allows one to make quick requests upon arriving at a Node of which
	//  LaneConnectors are available _from_ the segment you are arriving on.
	//NOTE: A multimap is another option here, but a map of vectors is probably easier on
	//      developers (who might not be familiar with multimaps).
	//      In addition, the multimap wouldn't be able to handle a uniqueness qualifier (which
	//      is why we use "set").
	std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> > connectors;

	///Bookkeeping: which RoadSegments meet at this Node?
	std::set<sim_mob::RoadSegment*> roadSegmentsAt;


friend class sim_mob::aimsun::Loader;

};





}
