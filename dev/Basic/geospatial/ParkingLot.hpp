/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>


namespace sim_mob
{


//Forward declarations
class RoadSegment;


/**
 * A simple parking lot.
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class ParkingLot {
public:
	///This lot's location. Tied to a RoadSegment and a laneID.
	///Note that all parking lots must have their own (small) RoadSegments which
	///  lead off from an Intersection and then to this ParkingLot. This allows
	///  cars leaving the lot to be queued up while waiting to exit.
	///
	///\todo
	///This becomes more complicated with single-directional lanes.
	std::pair<const RoadSegment*, unsigned int> location;


private:

};





}
