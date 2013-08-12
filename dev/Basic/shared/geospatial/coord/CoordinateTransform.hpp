//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "util/DynamicVector.hpp"

namespace sim_mob
{


/**
 * Helper class for representing lat/lng values
 */
struct LatLngLocation {
	LatLngLocation() : latitude(0), longitude(0) {}
	double latitude;
	double longitude;
};


/**
 * Base class for anything that can transform an (x,y) point to a (lat,lng) pair.
 *
 * \author Seth N. Hetu
 */
class CoordinateTransform {
public:
	virtual ~CoordinateTransform() {}
	virtual LatLngLocation transform(DPoint source) = 0;
};


}
