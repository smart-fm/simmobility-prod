/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once
#include "VehicleCounter.hpp"

namespace sim_mob {
sim_mob::VehicleCounter::VehicleCounter(const sim_mob::RoadSegment* rdSeg) : roadSegment(rdSeg)
{
	const std::vector<sim_mob::Lane*> lanes = rdSeg->getLanes();
	for(std::vector<sim_mob::Lane*>::const_iterator laneIt = lanes.begin();
			laneIt != lanes.end(); laneIt++)
	{
		movingVehicleCount[*laneIt] = 0;
		queuingVehicleCount[*laneIt] = 0;
	}
}

void sim_mob::VehicleCounter::incrementMovingCount(const sim_mob::Lane* lane, unsigned short movingCount) {
	movingVehicleCount[lane] += movingCount;
}
void sim_mob::VehicleCounter::incrementQueuingCount(const sim_mob::Lane* lane, unsigned short queuingCount) {
	queuingVehicleCount[lane] += queuingCount;
}

unsigned short sim_mob::VehicleCounter::getMovingVehicleCount(const sim_mob::Lane* lane) const {
	return movingVehicleCount.at(lane);
}

unsigned short sim_mob::VehicleCounter::getQueuingVehicleCount(const sim_mob::Lane* lane) const {
	return queuingVehicleCount.at(lane);
}

const sim_mob::RoadSegment* sim_mob::VehicleCounter::getRoadSegment() const {
	return roadSegment;
}

}
