/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "LaneGroup.hpp"

using namespace sim_mob;

sim_mob::LaneGroup::LaneGroup(const RoadSegment* parent, int id) : parentSegment(parent),lgID(id)
{

}

void sim_mob::LaneGroup::setLanes(std::vector<const sim_mob::Lane*> lanes)
{
	this->lanes = lanes;
}

int sim_mob::LaneGroup::getNumOfEmptySpaces(double length, double meanVehicleLength) const
{

}


std::vector<sim_mob::RoadSegment*> sim_mob::LaneGroup::getOutGoingSegments() {
	return outgoingRoadSegments;
}


void sim_mob::LaneGroup::setOutgoingSegments(std::vector<RoadSegment*> outRS)
{
	this->outgoingRoadSegments = outRS;
}

