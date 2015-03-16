/*
 * Polypoint.cpp
 *
 *  Created on: 16 Mar, 2015
 *      Author: max
 */

#include <geospatial/Polypoint.h>

namespace sim_mob {

Polypoint::Polypoint()
: id(-1),polylineId(-1),index(-1),x(0.0),y(0.0),z(0.0)
{
	// TODO Auto-generated constructor stub

}
Polypoint::Polypoint(const Polypoint& src)
: id(src.id),polylineId(src.polylineId),index(src.index),x(src.x),y(src.y),z(src.y)
{

}
Polypoint::~Polypoint() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
