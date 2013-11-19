//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <map>
#include <boost/tuple/tuple.hpp>

#include "geospatial/Node.hpp"
#include "util/LangHelpers.hpp"

namespace geo {
class UniNode_t_pimpl;
class GeoSpatial_t_pimpl;
}

namespace sim_mob {


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
 * A Node where two to four RoadSegments from within the same Link meet.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 *
 * This usually occurs because lane rules (or the actual number of lanes) change.
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
	/**
	 * Helper class representing the connection in a Lane-centric way. Note that the "from" lane is stored within UniNode.
	 */
	class UniLaneConnector {
	public:
		/**
		 * Build a new UniLaneConnector.
		 *    \param left The lane forward on the road but left of the current lane.
		 *    \param center The lane forward on the road and in the same location as the current lane.
		 *    \param right The lane forward on the road but right of the current lane.
		 */
		UniLaneConnector(const Lane* left=nullptr, const Lane* center=nullptr, const Lane* right=nullptr) : left(left), center(center), right(right) {}
		const Lane* left;
		const Lane* center;
		const Lane* right;
	};

	explicit UniNode(int x=0, int y=0) : Node(x, y),
		firstPair(nullptr, nullptr), secondPair(nullptr, nullptr)
	{}

	///Retrieve possible movement from a given Lane at this Node (left, right, or center)
	UniLaneConnector getForwardLanes(const sim_mob::Lane& from) const;

	///Retrieve the most likely forward movement from a given Lane at this Node.
	/// \return The center outgoing Lane, if it exists. Else, returns either left or right, if only ONE exists. Else, returns null.
	const Lane* getForwardDrivingLane(const sim_mob::Lane& from) const;

	///Retrieve the outgoing Lane at this Node.
	const sim_mob::Lane* getOutgoingLane(const sim_mob::Lane& from) const;
	std::vector<sim_mob::Lane*> getOutgoingLanes(Lane& from);
	const std::map<const sim_mob::Lane*, sim_mob::Lane* > & getConnectors() const {return connectors;}

	///Helper method: Build the connectors vector dynamically by aligning a lane in the "from" Road Segment with one
	/// in the "to" Road Segment.
	///NOTE: The "from/to" laneID pairs will definitely be cleaned up later; for now I'm just trying
	//       to get them to output something decent. At the moment they MUST correspond to "firstPair", "secondPair". ~Seth
	static void buildConnectorsFromAlignedLanes(UniNode* node, std::pair<unsigned int, unsigned int> fromToLaneIDs1, std::pair<unsigned int, unsigned int> fromToLaneIDs2);
	const std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*>& getRoadSegmentPair(bool first) const;
	const std::vector<const sim_mob::RoadSegment*>& getRoadSegments() const;

	//TODO: Temp:
	void setConnectorAt(const sim_mob::Lane* key, sim_mob::Lane* value) { this->connectors[key] = value; }
	void setNewConnectorAt(const sim_mob::Lane* key, boost::tuple<sim_mob::Lane*,sim_mob::Lane*,sim_mob::Lane*> values) { /*this->new_connectors[key] = value;*/ }


protected:
	//Helper, to keep our loop in order.
	static void buildConnectorsFromAlignedLanes(UniNode* node, const RoadSegment* fromSeg, const RoadSegment* toSeg, unsigned int fromAlignLane, unsigned int toAlignLane);
	static void buildNewConnectorsFromAlignedLanes(UniNode* node, const RoadSegment* fromSeg, const RoadSegment* toSeg, unsigned int fromAlignLane, unsigned int toAlignLane);

	//Old set of lane connectors
	std::map<const sim_mob::Lane*, sim_mob::Lane* > connectors;

	//New set of lane connectors.
	std::map<const Lane*, UniLaneConnector> new_connectors;

	///Bookkeeping: which RoadSegments meet at this Node?
	//  NOTE: If the RoadSegments in secondPair are null; then this is a one-way UniNode.
	//  As "from->to"

//TODO: Fix for xmlLoader
public:
	std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> firstPair;
	std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> secondPair;

protected:
	//Avoid iterating confusion
	mutable std::vector<const sim_mob::RoadSegment*> cachedSegmentsList;

	friend class sim_mob::aimsun::Loader;
	friend class ::geo::GeoSpatial_t_pimpl;



};



}
