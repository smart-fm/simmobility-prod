#pragma once

#include "RoadItem.hpp"


namespace sim_mob
{

/**
 * Representation of a defined space within a road segment that one may park in.
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class ParkingSlot : public sim_mob::RoadItem {
public:
	///Should represent "vertical" or "horizontal"
	unsigned int orientation;


private:



};





}
