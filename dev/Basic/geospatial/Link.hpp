/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <string>

//#include "RoadSegment.hpp"
#include "RoadItem.hpp"

#include "../constants.h"

namespace sim_mob
{

//Forward declarations
class RoadSegment;
class MultiNode;


namespace aimsun
{
//Forward declaration
class Loader;
} //End aimsun namespace



/**
 * A road or sidewalk. Generalized movement rules apply for agents inside a link,
 * which is itself composed of segments.
 */
class Link : public sim_mob::RoadItem {
public:
	///Return the length of this Link, which is the sum of all RoadSegments
	/// in the forward (if isForward is true) direction.
	int GetLength(bool isForward);

	///Return the RoadSegments which make up this Link, in either the forward
	/// (if isForward is true) or reverse direction.
	std::vector<sim_mob::RoadSegment*> GetPath(bool isForward);

	///The name of the particular segment. E.g., "Main Street 01".
	///Useful for debugging by location. May be auto-numbered.
	std::string getSegmentName(const sim_mob::RoadSegment* segment);

public:
	///The road link's name. E.g., "Main Street"
	std::string roadName;

protected:
	//TODO: This should probably be stored with some structure (to help with the
	//      single/bi-directional RoadSegment problem, and with naming).
	std::vector<sim_mob::RoadSegment*> segments;


friend class sim_mob::aimsun::Loader;


};





}
