/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "LaneConnector.hpp"

using std::pair;
using namespace sim_mob;

const pair<RoadSegment*, unsigned int>& sim_mob::LaneConnector::getLaneFrom() const
{
	return laneFrom;
}

const pair<RoadSegment*, unsigned int>& sim_mob::LaneConnector::getLaneTo() const
{
	return laneTo;
}

