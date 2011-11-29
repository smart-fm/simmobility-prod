/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "RoadItem.hpp"
#include "Point2D.hpp"


namespace sim_mob
{


/**
 * A Crossing is a RoadItem that crosses one (or more) RoadSegment(s).
 *
 * \note
 * Currently, the "start" and "end" points of a Crossing don't make much sense; the crossing
 * already has a "far" and "near" line.
 */
class Crossing : public RoadItem {
public:
	Crossing() : RoadItem() {}


//protected:
	//The line (start/end points that make up the line) "near" the intersection
	std::pair<sim_mob::Point2D, sim_mob::Point2D> nearLine;

	//The line that is "far" from the intersection (further down the road)
	std::pair<sim_mob::Point2D, sim_mob::Point2D> farLine;

};





}
