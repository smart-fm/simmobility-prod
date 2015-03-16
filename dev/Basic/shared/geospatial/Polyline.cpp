//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include <geospatial/Polyline.h>

namespace sim_mob {

Polyline::Polyline()
:type(POLYLINE_TYPE_POLYPOINT),id(-1),length(0.0)
{
	// TODO Auto-generated constructor stub

}
Polyline::Polyline(const Polyline& src)
:id(src.id),type(src.type),length(src.length),scenario(src.scenario)
{

}
Polyline::~Polyline() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
