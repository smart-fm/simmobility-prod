/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <stdexcept>

#include "util/OpaqueProperty.hpp"
#include "Pavement.hpp"
#include "Link.hpp"
namespace geo
{
class segment_t_pimpl;
class Segments_pimpl;
}
namespace sim_mob
{


//Forward declarations
class Lane;
class BusStop;
class RoadNetworkPackageManager;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

namespace aimsun
{
//Forward declaration
class Loader;
class LaneLoader;
} //End aimsun namespace



/**
 * Part of a Link with consistent lane numbering. RoadSegments may be bidirectional.
 *
 * \author Seth N. Hetu
 * \author Matthew Bremer Bruchon
 * \author Xu Yan
 * \author LIM Fung Chai
 */
class RoadSegment : public sim_mob::Pavement {
public:
	///Create a RoadSegment as part of a given Link.
	RoadSegment(){}//needed by xml reader --vahid
	explicit RoadSegment(sim_mob::Link* parent);
	
	explicit RoadSegment(sim_mob::Link* parent, unsigned long id);
	const unsigned long  & getSegmentID()const ;

	bool operator== (const RoadSegment* rhs) const
	{
		   return (rhs->getStart()==this->getStart())&&(rhs->getEnd()==this->getEnd());
	}
	///Return the Link this RoadSegment is part of.
	sim_mob::Link* getLink() const { return parentLink; }

	///Retrieve the Lanes within this segment.
	//TEMP: For now, returning a const vector of non-const lanes. Will fix later. ~Seth
	void end1(){};
	const std::vector<sim_mob::Lane*>& getLanes() const {
		void end1();
		return lanes; }


	sim_mob :: BusStop* getBusStop() {
			return busstop; }

	///Retrieve whether this is a single or bidirectional Road Segment.
	bool isSingleDirectional();
	bool isBiDirectional();

	///Translate an array index into a useful lane ID and a set of properties.
	std::pair<int, const sim_mob::Lane*> translateRawLaneID(unsigned int rawID);

	///Return the polyline of an individual lane. May be cached in lanePolylines_cached. May also be precomputed, and stored in lanePolylines_cached.
	const std::vector<sim_mob::Point2D>& getLaneEdgePolyline(unsigned int laneID) /*const*/;

	//Force expansion of all Lane and LaneEdge polylines
	void syncLanePolylines();/* const;*/

	//RoadSegments may have hidden properties useful only in for the visualizer.
	OpaqueProperty<int> originalDB_ID;

#ifndef SIMMOB_DISABLE_MPI
	///The identification of RoadSegment is packed using PackageUtils;
	static void pack(PackageUtils& package, const RoadSegment* one_segment);

	///UnPackageUtils use the identification of RoadSegment to find the RoadSegment Object
	static const RoadSegment* unpack(UnPackageUtils& unpackage);
#endif

public:
	///Maximum speed of this road segment.
	unsigned int maxSpeed;

	///TODO This should be made private again.
	mutable std::vector< std::vector<sim_mob::Point2D> > laneEdgePolylines_cached;
	void setLanes(std::vector<sim_mob::Lane*>);
private:
	///Collection of lanes. All road segments must have at least one lane.
	std::vector<sim_mob::Lane*> lanes;
	sim_mob::BusStop* busstop;
	//int getBustStopID;
	///Computed polylines are cached here.
	///These run from 0 (for the median) to lanes.size()+1 (for the outer edge).
	void specifyEdgePolylines(const std::vector< std::vector<sim_mob::Point2D> >& calcdPolylines);
	void makeLanePolylineFromEdges(sim_mob::Lane* lane, const std::vector<sim_mob::Point2D>& inner, const std::vector<sim_mob::Point2D>& outer) const;
	std::vector<sim_mob::Point2D> makeLaneEdgeFromPolyline(sim_mob::Lane* refLane, bool edgeIsRight) const;

	//mutable std::vector< std::vector<sim_mob::Point2D> > laneEdgePolylines_cached;

	///Helps to identify road segments which are bi-directional.
	///We count lanes from the LHS, so this doesn't change with drivingSide
	unsigned int lanesLeftOfDivider;

	///Which link this appears in
	sim_mob::Link* parentLink;
//	std::string segmentID;
	unsigned long segmentID;

friend class sim_mob::aimsun::Loader;
friend class sim_mob::aimsun::LaneLoader;
friend class sim_mob::RoadNetworkPackageManager;
friend class ::geo::segment_t_pimpl;
friend class ::geo::Segments_pimpl;

};





}
