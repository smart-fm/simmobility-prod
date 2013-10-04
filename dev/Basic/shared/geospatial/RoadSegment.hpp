//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>

#include "conf/settings/DisableMPI.h"

#include "util/OpaqueProperty.hpp"
#include "geospatial/Pavement.hpp"


namespace geo {
class segment_t_pimpl;
class Segments_pimpl;
class link_t_pimpl;
}

namespace sim_mob
{
//Forward declarations
class Lane;
class BusStop;
class RoadNetworkPackageManager;
class Conflux;

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

/*
 * SpeedDensityParams is the place holder for storing the parameters of the
 * speed density function for this road segment.
 * \author Harish
 */
struct SupplyParams {
	double freeFlowSpeed;  ///<Maximum speed of the road segment
	double jamDensity;     ///<density during traffic jam in vehicles / m
	double minDensity;     ///<minimum traffic density in vehicles / m
	double minSpeed;       ///<minimum speed in the segment
	double capacity;       ///<segment capacity in vehicles/second
	double alpha;          ///<Model parameter of speed density function
	double beta;           ///<Model parameter of speed density function


	SupplyParams(double maxSpeed, double minSpeed, double maxDensity, double minDensity, double capacity, double a, double b)
		: freeFlowSpeed(maxSpeed), jamDensity(maxDensity), minDensity(minDensity), minSpeed(minSpeed), capacity(capacity), alpha(a), beta(b)
	{}
};

/**
 * Part of a Link with consistent lane numbering. RoadSegments are unidirectional.
 *
 * \author Seth N. Hetu
 * \author Matthew Bremer Bruchon
 * \author Xu Yan
 * \author LIM Fung Chai
 */
class RoadSegment : public sim_mob::Pavement {
public:
	///Create a RoadSegment as part of a given Link.
	//explicit RoadSegment(sim_mob::Link* paren=nullptr);

	//TODO: Some of these are only used by the geo* classes; need to re-think.
	void setParentLink(sim_mob::Link* parent);
	void setID(unsigned long id) { this->segmentID = id; }
	//void setLanes(const std::vector<sim_mob::Lane*>& ln) { this->lanes = ln; }
	void setStart(sim_mob::Node* st) { this->start = st; }
	void setEnd(sim_mob::Node* en) { this->end = en; }
	std::string getStartEnd() const;

public:
	explicit RoadSegment(sim_mob::Link* parent=nullptr, const SupplyParams* sParams=nullptr, unsigned long id=-1) :
		Pavement(),
		maxSpeed(0), capacity(0), busstop(nullptr), lanesLeftOfDivider(0), parentLink(parent),segmentID(id),
		supplyParams(sParams), parentConflux(nullptr), laneZeroLength(-1.0)
	{}

	const unsigned long  getSegmentID()const ;
	const unsigned int getLanesLeftOfDivider() const { return lanesLeftOfDivider; }

	bool operator== (const RoadSegment* rhs) const
	{
		   return (rhs->getStart()==this->getStart())&&(rhs->getEnd()==this->getEnd());
	}
	///Return the Link this RoadSegment is part of.
	sim_mob::Link* getLink() const { return parentLink; }

	///Retrieve the Lanes within this segment.
	//TEMP: For now, returning a const vector of non-const lanes. Will fix later. ~Seth
	const std::vector<sim_mob::Lane*>& getLanes() const {
		return lanes;
	}

	///Return the Lane at a given ID, or null if that Lane ID is out of bounds.
	const sim_mob::Lane* getLane(int laneID) const;

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
	double capacity;
	///TODO This should be made private again.
	mutable std::vector<std::vector<sim_mob::Point2D> > laneEdgePolylines_cached;
	void setLanes(std::vector<sim_mob::Lane*>);

	//TODO: Added for xmlLoader
	void setLanesLeftOfDivider(unsigned int val) { lanesLeftOfDivider = val; }

	const sim_mob::SupplyParams* getSupplyParams() {
		return supplyParams;
	}

	sim_mob::Conflux* getParentConflux() const {
		return parentConflux;
	}

	double getLengthOfSegment();

	void setParentConflux(sim_mob::Conflux* parentConflux) {
		this->parentConflux = parentConflux;
	}

	double computeLaneZeroLength() const;

	void setCapacity(); //for now since the capacity is not loaded from the xml

	const double getLaneZeroLength() const{
		return laneZeroLength;
	}
	/*void initLaneGroups() const;
	 void groupLanes(std::vector<sim_mob::RoadSegment*>::const_iterator rdSegIt, const std::vector<sim_mob::RoadSegment*>& segments, sim_mob::Node* start, sim_mob::Node* end) const;
	 void matchLanes(std::map<const sim_mob::Lane*, std::vector<RoadSegment*> >& mapRS) const;*/

private:
	///Collection of lanes. All road segments must have at least one lane.
	std::vector<sim_mob::Lane*> lanes;
	sim_mob::BusStop* busstop;
	//int getBustStopID;
	///Computed polylines are cached here.
	///These run from 0 (for the median) to lanes.size()+1 (for the outer edge).
	void specifyEdgePolylines(const std::vector<std::vector<sim_mob::Point2D> >& calcdPolylines);
	void makeLanePolylineFromEdges(sim_mob::Lane* lane, const std::vector<sim_mob::Point2D>& inner, const std::vector<sim_mob::Point2D>& outer) const;
	std::vector<sim_mob::Point2D> makeLaneEdgeFromPolyline( sim_mob::Lane* refLane, bool edgeIsRight) const;

	///Helps to identify road segments which are bi-directional.
	///We count lanes from the LHS, so this doesn't change with drivingSide
	unsigned int lanesLeftOfDivider;

	///Which link this appears in
	sim_mob::Link* parentLink;

	/// Conflux to which this segment belongs to
	mutable sim_mob::Conflux* parentConflux;

	unsigned long segmentID;

	const sim_mob::SupplyParams* supplyParams;

	double laneZeroLength;

	friend class sim_mob::aimsun::Loader;
	friend class sim_mob::aimsun::LaneLoader;
	friend class sim_mob::RoadNetworkPackageManager;
	friend class geo::segment_t_pimpl;
	friend class geo::Segments_pimpl;
	friend class geo::link_t_pimpl;

};





}
