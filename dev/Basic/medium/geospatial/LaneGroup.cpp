/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "LaneGroup.hpp"

using namespace sim_mob;

sim_mob::medium::LaneGroup::LaneGroup(RoadSegment* parent, unsigned long id) : parentSegment(parent),lgID(id)
{

}

void sim_mob::medium::LaneGroup::setLanes(std::vector<sim_mob::Lane*> lanes)
{
	this->lanes = lanes;
}

int sim_mob::medium::LaneGroup::getNumOfEmptySpaces(double length, double meanVehicleLength) const
{

}
