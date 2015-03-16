//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#ifndef GEOSPATIAL_TURNINGPOLYLINE_H_
#define GEOSPATIAL_TURNINGPOLYLINE_H_

#include <geospatial/aimsun/Polyline.hpp>

namespace sim_mob {

class TurningPolyline: public aimsun::Polyline {
public:
	TurningPolyline();
	int turningId;
	TurningSection *turning;
	virtual ~TurningPolyline();
};

} /* namespace sim_mob */

#endif /* GEOSPATIAL_TURNINGPOLYLINE_H_ */
