#pragma once

#include <vector>

#include "Pavement.hpp"
//#include "Lane.hpp"

namespace sim_mob
{


//Forward declarations
class Lane;



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

	//Translate an array index into a useful lane ID and a set of properties.
	std::pair<int, const Lane&> translateRawLaneID(unsigned int ID) { return std::pair<int, const Lane&>; }


private:
	std::vector<const sim_mob::Lane*> lanes;
	unsigned int lanesLeftOfDivider; //We count lanes from the LHS, so this doesn't change with drivingSide


};





}
