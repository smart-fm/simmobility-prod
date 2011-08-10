/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <map>
#include <set>

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
 * A location on a map where other elements interact. Nodes contain a Point2D representing their location,
 * and include information on the Lane Connectors and RoadSegments at that particular location.
 */
class Node {
public:

	///Query the list of connectors at the current node, restricting the results to
	///   those which originate at the "from" segment.
	///Fails if no outgoing Lanes exist.
	///The reference to this vector may be invalidated when a new connector is added
	///   to this Node which links from a Lane* that is not currently linked.
	const std::set<sim_mob::LaneConnector*>& getOutgoingLanes(const sim_mob::RoadSegment& from) const;

	///Test if connetors exist at this node.
	bool hasOutgoingLanes(const sim_mob::RoadSegment& from) const;

	///Return a list of all Road Segments at this location.
	///fromHere and toHere are used to modify the search.
	//NOTE: We already have this.... but maybe from/to might be useful later?
	//std::vector<sim_mob::RoadSegment*> getAllRoadSegments(bool fromHere, bool toHere) const;

	///Retrieve a list of all RoadSegments at this node.
	const std::set<sim_mob::RoadSegment*>& getRoadSegments() const { return roadSegmentsAt; }

	//This node's location.
	sim_mob::Point2D* location;


protected:
	///Helper: Add a Lane Connector mapping. Handles bookkeeping
	//NOTE: This is not actually how RoadSegments are saved.
	//void addLaneConnector(sim_mob::Lane* from, sim_mob::Lane* to);

	///Mapping from RoadSegment* -> set<LaneConnector*> representing lane connectors.
	///Currently allows one to make quick requests upon arriving at a Node of which
	//  LaneConnectors are available _from_ the segment you are arriving on.
	//NOTE: A multimap is another option here, but a map of vectors is probably easier on
	//      developers (who might not be familiar with multimaps).
	//      In addition, the multimap wouldn't be able to handle a uniqueness qualifier (which
	//      is why we use "set").
	std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> > connectors;
	//std::multimap<sim_mob::RoadSegment*, sim_mob::RoadSegment*> connectors;

	///Bookkeeping: which RoadSegments meet at this Node?
	std::set<sim_mob::RoadSegment*> roadSegmentsAt;


friend class sim_mob::aimsun::Loader;

};





}
