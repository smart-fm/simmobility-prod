//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Crossing.hpp"
#include <boost/multi_index_container.hpp>


#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "util/GeomHelpers.hpp"

using namespace sim_mob;

/*RoadSegment* sim_mob::Crossing::getRoadSegment() const
{
	return roadSegment;
};


void sim_mob::Crossing::setRoadSegment(RoadSegment *rs)
{
	//TODO: Evaluate why we can't set the RoadSegment to a null value.
	if(rs) {
		roadSegment = rs;
	}
}*/


