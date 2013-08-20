//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RoadItem.hpp"

#include "geospatial/RoadSegment.hpp"

using namespace sim_mob;


unsigned long sim_mob::RoadItem::generateRoadItemID(const sim_mob::RoadSegment& rs)
{
	return rs.getSegmentID() * 10 + rs.obstacles.size();
}


//????
/*void sim_mob::RoadItem::setParentSegment(sim_mob::RoadSegment*)
{
};*/
