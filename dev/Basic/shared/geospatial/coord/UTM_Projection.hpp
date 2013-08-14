//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "geospatial/coord/CoordinateTransform.hpp"

namespace sim_mob
{

/**
 * Allowed coordinate systems
 */
enum CoordSystem {
	COORD_WGS84,
};

/**
 * UTM zones.
 *
 * \todo Add them all in later.
 */
enum UTM_Zones {
	UTM_INVALID,
	UTM_48N,
};


/**
 * Represents a UTM projection. Contains a coordinate system (usually WGS 84) and a utm zone.
 *
 * \author Seth N. Hetu
 */
class UTM_Projection : public CoordinateTransform {
public:
	UTM_Projection() : coordSys(COORD_WGS84), utmZone(UTM_INVALID)
	{}

	virtual LatLngLocation transform(DPoint source) {
		throw std::runtime_error("Coordinate transform not yet supported.");
	}

	CoordSystem coordSys;
	UTM_Zones utmZone;
};


}
