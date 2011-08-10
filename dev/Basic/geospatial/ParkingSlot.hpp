/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "RoadItem.hpp"


namespace sim_mob
{

/**
 * Representation of a defined space within a road segment that one may park in.
 */
class ParkingSlot : public sim_mob::RoadItem {
public:
	///TODO: Should represent "vertical" or "horizontal"
	unsigned int orientation;


private:



};





}
