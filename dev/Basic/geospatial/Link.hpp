/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <set>
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
	//Initialize a link with the given set of segments
	void initializeLinkSegments(const std::set<sim_mob::RoadSegment*>& segments);

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
	//List of pointers to RoadSegments in each direction
	std::vector<sim_mob::RoadSegment*> fwdSegments;
	std::vector<sim_mob::RoadSegment*> revSegments;



friend class sim_mob::aimsun::Loader;


};





}
