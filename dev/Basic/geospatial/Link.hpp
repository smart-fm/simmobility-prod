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


/**
 * A road or sidewalk. Generalized movement rules apply for agents inside a link,
 * which is itself composed of segments.
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class Link : public sim_mob::RoadItem {
public:
	int GetLength(bool isForward) { return 0; }
	std::vector<sim_mob::RoadSegment*> GetPath(bool isForward) { return std::vector<sim_mob::RoadSegment*>(); }


	//The name of the particular segment. E.g., "Main Street 01".
	//Useful for debugging by location. May be auto-numbered.
	std::string getSegmentName(const sim_mob::RoadSegment* segment) { return ""; }

public:
	///The road link's name. E.g., "Main Street"
	std::string roadName;

protected:
	std::vector<sim_mob::RoadSegment*> segments;


};





}
