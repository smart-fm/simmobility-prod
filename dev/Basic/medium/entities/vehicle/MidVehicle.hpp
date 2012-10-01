/* Copyright Singapore-MIT Alliance for Research and Technology */
#pragma once

#include "entities/vehicle/Vehicle.hpp"

namespace sim_mob {

namespace medium {

class MidVehicle : public sim_mob::Vehicle {
public:
	MidVehicle(std::vector<sim_mob::WayPoint> wp_path, int startLaneID);
	MidVehicle(std::vector<sim_mob::WayPoint> wp_path, int startLaneID, double length, double width);

	~MidVehicle();
	double getDistanceMovedInSegment() const;   //re-defining this to suit mid-term
	void setDistanceMovedInSegment(double distance);

	/*getPosition will return the distance from the end of the road segment
	 * for queuing vehicles this will be the length of the queue before the vehicle
	 */
	double getPosition() const;
	void setPosition(double distToEndSeg);
	bool isQueuing;

private:
	double distMovedInCurrSegment;
	double posInRoadSeg;
};

}
}

