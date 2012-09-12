/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"

namespace sim_mob {

class VehicleCounter {

private:
	const sim_mob::RoadSegment* roadSegment;
	std::map<const sim_mob::Lane*, unsigned short> movingVehicleCount;
	std::map<const sim_mob::Lane*, unsigned short> queuingVehicleCount;

public:
	VehicleCounter(const sim_mob::RoadSegment* rdSeg);
	void incrementMovingCount(const sim_mob::Lane* lane, unsigned short movingCount);
	void incrementQueuingCount(const sim_mob::Lane* lane, unsigned short queuingCount);
	unsigned short getMovingVehicleCount(const sim_mob::Lane* lane) const;
	unsigned short getQueuingVehicleCount(const sim_mob::Lane* lane) const;
	const sim_mob::RoadSegment* getRoadSegment() const;
};

}
