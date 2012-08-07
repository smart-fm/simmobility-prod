/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "util/LangHelpers.hpp"

#include "Lane.hpp"

namespace geo
{
class connector_t_pimpl;
class UniNode_t_pimpl;
}

namespace sim_mob
{


//Forward declarations
class RoadSegment;


namespace aimsun
{
//Forward declaration
class Loader;
} //End namespace aimsun


/**
 * A lane for motorized vehicles. Links one Road Segment to another, by Lane ID.
 * \author Seth N. Hetu
 */
class LaneConnector {
public:
	LaneConnector() : laneFrom(nullptr), laneTo(nullptr) {}

	const sim_mob::Lane* getLaneFrom() const {
		return laneFrom;
	}
	const sim_mob::Lane* getLaneTo() const {
		return laneTo;
	}

private:
	//NOTE: These items used to be const, but it's easier to declare them private and just
	//      return a const item.
	sim_mob::Lane* laneFrom;
	sim_mob::Lane* laneTo;


friend class sim_mob::aimsun::Loader;
friend class ::geo::connector_t_pimpl;
friend class ::geo::UniNode_t_pimpl;


};





}
