/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "RoadSegment.hpp"

#include "Lane.hpp"

using namespace sim_mob;

using std::pair;
using std::vector;


sim_mob::RoadSegment::RoadSegment(Link* parent) : Pavement(), parentLink(parent)
{

}


bool sim_mob::RoadSegment::isSingleDirectional()
{
	return lanesLeftOfDivider==0 || lanesLeftOfDivider==lanes.size()-1;
}


bool sim_mob::RoadSegment::isBiDirectional()
{
	return !isSingleDirectional();
}


pair<int, const Lane*> sim_mob::RoadSegment::translateRawLaneID(unsigned int rawID)
{
	//TODO: Need to convert the ID into an "effective" lane ID based on road direction
	//      (including bidirectional segments).
	throw std::runtime_error("Not yet defined.");
}


const vector<Point2D>& sim_mob::RoadSegment::getLanePolyline(unsigned int laneID) const
{
	//Expand the cached vector. Only do this ONCE, so that references are never invalidated.
	if (lanePolylines_cached.size() != lanes.size()+1) {
		if (!lanePolylines_cached.empty()) {
			//NOTE: I'm throwing an exception for now. We need to decide error handling in our framework
			//      sometime soon. ~Seth
			throw std::runtime_error("Attempting to resize active polyline cache.");
		}
		lanePolylines_cached.resize(lanes.size(), vector<Point2D>());
	}

	//Now, rebuild the polylines as needed. We are guaranteed that the minimum number of points
	//     in a polyline will be 2 (start and end).
	if (lanePolylines_cached[laneID].empty()) {
		//TODO: Offset and build the polyline.
		throw std::runtime_error("not_implemented");
	}

	return lanePolylines_cached[laneID];
}


