//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>

#include "geospatial/coord/CoordinateTransform.hpp"

namespace sim_mob
{

///A simple class for representing Road Runner regions. Points are in some order (clockwise?) in Lat/Lng format.
class RoadRunnerRegion {
public:
	RoadRunnerRegion() : id(0) {}

	int id;
	std::vector<sim_mob::LatLngLocation> points;
};


}
