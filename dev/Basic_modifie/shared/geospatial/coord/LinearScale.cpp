//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "LinearScale.hpp"

using namespace sim_mob;

//TODO: This is very untested.
LatLngLocation sim_mob::LinearScale::transform(Point source)
{
	//Ensure our source point is within the valid "source" bounds.
	if (source.getX() < sourceX.min || source.getX() > sourceX.max || source.getY() < sourceY.min || source.getY() > sourceY.max) {
		//We can't throw an error, so just return the center of the destination zone.
		Warn() <<"LinearScale::transform() -> Point is outside the stated scale for this transform.";
		return LatLngLocation(
			destLatitude.min  + destLatitude.size()/2,
			destLongitude.min + destLongitude.size()/2
		);
	}

	//Else, perform a simple scaling operation.
	double percX = (source.getX() - sourceX.min) / sourceX.size();
	double percY = (source.getY() - sourceY.min) / sourceY.size();

	//...accounting for a flipped latitude, of course.
	double lat = (1.0-percY) * destLatitude.size() + destLatitude.min;
	double lng = percX * destLongitude.size() + destLongitude.min;
	return LatLngLocation(lat, lng);
}
