/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <stdexcept>

#include "Pavement.hpp"
#include "Link.hpp"

namespace sim_mob
{


//Forward declarations
class Lane;

namespace aimsun
{
//Forward declaration
class Loader;
} //End aimsun namespace



/**
 * Part of a Link with consistent lane numbering. RoadSegments may be bidirectional.
 */
class RoadSegment : public sim_mob::Pavement {
public:
	RoadSegment(sim_mob::Link* parent);

	///Return the Link this RoadSegment is part of.
	sim_mob::Link* getLink() { return parentLink; }

	const std::vector<const sim_mob::Lane*>& getLanes() const { return lanes; }

	bool isSingleDirectional();
	bool isBiDirectional();

	///Translate an array index into a useful lane ID and a set of properties.
	std::pair<int, const sim_mob::Lane*> translateRawLaneID(unsigned int rawID);

	///Return the polyline of an individual lane. May be cached in lanePolylines_cached. May also be precomputed, and stored in lanePolylines_cached.
	const std::vector<sim_mob::Point2D>& getLanePolyline(unsigned int laneID);


public:
	///Maximum speed of this road segment.
	int maxSpeed;

	void* roadBulge; ///<Currently haven't decided how to represent bulge.


private:
	///Collection of lanes. All road segments must have at least one lane.
	std::vector<const sim_mob::Lane*> lanes;

	///Computed polylines are cached here.
	std::vector< std::vector<sim_mob::Point2D> > lanePolylines_cached;

	///Helps to identify road segments which are bi-directional.
	///We count lanes from the LHS, so this doesn't change with drivingSide
	unsigned int lanesLeftOfDivider;

	///Widths of each lane. If this vector is empty, each lane's width is an even division of
	///Pavement::width() / lanes.size()
	//std::vector<unsigned int> laneWidths;

	///Which link this appears in
	sim_mob::Link* parentLink;

friend class sim_mob::aimsun::Loader;


};





}
