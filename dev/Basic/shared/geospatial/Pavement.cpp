//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Pavement.hpp"

#include <stdexcept>
#include "util/OutputUtil.hpp"

using namespace sim_mob;


void sim_mob::Pavement::addObstacle(centimeter_t offset, const RoadItem* item, bool fixErrors)
{
	//Too small?
	if (offset<0) {
		if (!fixErrors) { throw std::runtime_error("Can't add obstacle; offset is less than zero."); }
		LogOut("Fixing RoadItem offset: " <<offset <<" to: " <<0 <<std::endl);
		offset = 0;
	}
	//Too big?
	if (offset>length) {
		if (!fixErrors) { throw std::runtime_error("Can't add obstacle; offset is greater than the segment length."); }
		if (length==0) { throw std::runtime_error("Can't fix Road Segment obstacle; length has not been set."); }
		LogOut("Fixing RoadItem offset: " <<offset <<" to: " <<length <<std::endl);
		offset = length;
	}
	//Already something there?
	if (obstacles.count(offset)>0) {
		//For now we can't fix it.
		//TODO: There's multiple fixes; just pick one.
		throw std::runtime_error("Can't add obstacle; something is already at that offset.");
	}

	//Add it.
	obstacles[offset] = item;
}


RoadItemAndOffsetPair sim_mob::Pavement::nextObstacle(const Point2D& pos, bool isForward) const
{
	throw std::runtime_error("Not implemented yet.");
}


RoadItemAndOffsetPair sim_mob::Pavement::nextObstacle(centimeter_t offset, bool isForward) const
{

	//Simple!  //yes simple. what if the offset = it->first = 0 ?   :) vahid

	if(isForward == true)
	for (std::map<int, const RoadItem*>::const_iterator it=obstacles.begin(); it!=obstacles.end(); it++) {
			if (it->first >= offset) {
				return RoadItemAndOffsetPair(it->second, it->first);
		}
	}
	else

	if(isForward == false)
	for (std::map<int, const RoadItem*>::const_reverse_iterator it=obstacles.rbegin(); it!=obstacles.rend(); it++) {
			if (it->first <= offset) {
				return RoadItemAndOffsetPair(it->second, it->first);
		}
	}


	return RoadItemAndOffsetPair(nullptr, 0);
}

void sim_mob::Pavement::GeneratePolyline(Pavement* p, Point2D center, double bulge, int segmentLength)
{
	throw std::runtime_error("Not implemented yet.");
}

