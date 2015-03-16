//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#ifndef GEOSPATIAL_TURNINGPOLYLINE_H_
#define GEOSPATIAL_TURNINGPOLYLINE_H_

#include "geospatial/Polyline.h"
#include "geospatial/Polypoint.h"
#include "geospatial/TurningSection.hpp"

namespace sim_mob {

class TurningSection;
class Polyline;

class TurningPolyline: public Polyline {
public:
	TurningPolyline();
	TurningPolyline(const TurningPolyline& tp);
	int turningId;
	TurningSection *turning;
	std::vector<Polypoint*> polypoints;
	virtual ~TurningPolyline();
};

} /* namespace sim_mob */

#endif /* GEOSPATIAL_TURNINGPOLYLINE_H_ */
