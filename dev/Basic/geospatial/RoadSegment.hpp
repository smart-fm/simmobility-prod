#pragma once

#include <vector>

#include "Pavement.hpp"
#include "Lane.hpp"

namespace sim_mob
{


//Forward declarations
//class Lane;



/**
 * Part of a Link with consistent lane numbering. RoadSegments may be bidirectional
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class RoadSegment : public sim_mob::Pavement {
public:
	bool isSingleDirectional() {
		return lanesLeftOfDivider==0 || lanesLeftOfDivider==lanes.size()-1;
	}
	bool isBiDirectional() {
		return !isSingleDirectional();
	}

	///Translate an array index into a useful lane ID and a set of properties.
	std::pair<int, const Lane&> translateRawLaneID(unsigned int ID) { Lane l; return std::pair<int, const Lane&>(1, l); }

	///Return the polyline of an individual lane. May be cached in lanePolylines_cached. May also be precomputed, and stored in lanePolylines_cached.
	std::vector<sim_mob::Point2D>& getLanePolyline(unsigned int laneID) { std::vector<sim_mob::Point2D> res; return res; }


public:
	///Maximum speed of this road segment.
	unsigned int maxSpeed;

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
	std::vector<unsigned int> laneWidths;


};





}
