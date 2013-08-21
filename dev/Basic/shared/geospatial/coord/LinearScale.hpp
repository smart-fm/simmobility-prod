//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "geospatial/coord/CoordinateTransform.hpp"

namespace sim_mob
{


/**
 * Represents a simpler Linear scaling useful for maps generated from random data.
 *
 * \author Seth N. Hetu
 */
class LinearScale : public CoordinateTransform {
public:
	///Helper class: represent a range from min to max inclusive
	struct Range {
		Range() : min(0), max(0) {}
		double min;
		double max;
		double size() { return max-min; }
	};

	LinearScale()
	{}

	virtual LatLngLocation transform(DPoint source);

	Range sourceX;
	Range sourceY;
	Range destLatitude;
	Range destLongitude;
};


}
