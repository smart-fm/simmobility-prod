//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#ifndef GEOSPATIAL_POLYLINE_H_
#define GEOSPATIAL_POLYLINE_H_

enum PolylineType
{
	POLYLINE_TYPE_POLYPOINT = 0,
	POLYLINE_TYPE_QUAD_BEZIER_CURVE = 1,
	POLYLINE_TYPE_SVGPATH = 2,
	POLYLINE_TYPE_MITSIM    = 3
};

namespace sim_mob {

class Polyline {
public:
	Polyline();

	PolylineType type;
	virtual ~Polyline();
};

} /* namespace sim_mob */

#endif /* GEOSPATIAL_POLYLINE_H_ */
