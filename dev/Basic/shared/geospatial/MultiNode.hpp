/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <map>
#include <set>

#include "GenConfig.h"
#include "geospatial/RoadSegment.hpp"
#include "Node.hpp"
/*namespace geo {
class intersection_t_pimpl;
class GeoSpatial_t_pimpl;
}*/

namespace sim_mob
{


//Forward declarations
class Point2D;
class Link;
class Lane;
class LaneConnector;
class RoadSegment;
class RoadNetwork;

namespace aimsun
{
//Forward declarations
class Loader;
}


/**
 * A Node where any number of Links meet, usually representing an Interserction or Roundabout.
 * Each Link contributes one RoadSegment to this MultiNode, and each Lane of each RoadSegment
 * may connect to any number of lanes from any number of other RoadSegments.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 */
struct Sorter
{
	bool operator()(sim_mob::RoadSegment *a , sim_mob::RoadSegment *b)
	{
		return (a->getSegmentID() < b->getSegmentID());
	}
};
class MultiNode : public sim_mob::Node {
public:
	MultiNode(int x, int y) : Node(x, y) {}

	///Query the list of connectors at the current node, restricting the results to
	///   those which originate at the "from" segment.
	///Fails if no outgoing Lanes exist.
	///The reference to this vector may be invalidated when a new connector is added
	///   to this Node which links from a Lane* that is not currently linked.
	const std::set<sim_mob::LaneConnector*>& getOutgoingLanes(const sim_mob::RoadSegment* from) const;

	///Test if connetors exist at this node.
	bool hasOutgoingLanes(const sim_mob::RoadSegment* from) const;

	///Retrieve a list of all RoadSegments at this node.
	const std::set<sim_mob::RoadSegment*>& getRoadSegments() const { return roadSegmentsAt; }
	bool canFindRoadSegment(sim_mob::RoadSegment* rs) const;
	//Determine which RoadSegments (and in which direction) you have to cross as a pedestrian approaching this
	//  MultiNode
	//TODO: The return value will _definitely_ be a class.
	std::pair< std::vector< std::pair<sim_mob::RoadSegment*, bool> >, std::vector< std::pair<sim_mob::RoadSegment*, bool> > >
		getPedestrianPaths(const sim_mob::Node* const nodeBefore, const sim_mob::Node* const nodeAfter) const;

	//seth suggested its removal for good reasons-vahid
//	static void BuildClockwiseLinks(const sim_mob::RoadNetwork& rn, sim_mob::MultiNode* node);
	const std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> > & getConnectors() const {return connectors;}
	const std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> > & getConnectors()  {return connectors;}

	//TEMP: Added for XML loading.
	void setConnectorAt(const sim_mob::RoadSegment* key, std::set<sim_mob::LaneConnector*>& val) { this->connectors[key] = val; }
	void addRoadSegmentAt(sim_mob::RoadSegment* rs) { roadSegmentsAt.insert(rs); }
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

	//Bookkeeping: Store a list of RoadSegments in "clockwise" (can be counter-clockwise for rhs) order,
	//             along with a flag of whether or not this segment is "forward". Used to determine which
	//             Segments pedestrians must cross when approaching the intersection.
public: //TEMP
	std::vector< std::pair<RoadSegment*, bool> > roadSegmentsCircular;


friend class sim_mob::aimsun::Loader;

};





}
