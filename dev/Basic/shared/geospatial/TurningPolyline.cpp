//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)
#include <geospatial/TurningPolyline.h>

namespace sim_mob {

TurningPolyline::TurningPolyline()
:Polyline(),turningId(-1),turning(nullptr)
{
	// TODO Auto-generated constructor stub

}
TurningPolyline::TurningPolyline(const sim_mob::TurningPolyline& tp)
:Polyline(tp),turning(tp.turning),turningId(tp.turningId)
{

}

TurningPolyline::~TurningPolyline() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
