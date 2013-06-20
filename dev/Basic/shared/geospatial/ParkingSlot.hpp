//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "RoadItem.hpp"


namespace sim_mob
{

/**
 * Representation of a defined space within a road segment that one may park in.
 * \author Seth N. Hetu
 */
class ParkingSlot : public sim_mob::RoadItem {
public:
	ParkingSlot() : RoatItem() {}

	///TODO: Should represent "vertical" or "horizontal"
	unsigned int orientation;


private:



};





}
