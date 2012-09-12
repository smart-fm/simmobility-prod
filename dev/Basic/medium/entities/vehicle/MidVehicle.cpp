/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "MidVehicle.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;
using std::vector;

sim_mob::medium::MidVehicle::MidVehicle(std::vector<sim_mob::WayPoint> wp_path, int startLaneID)
: Vehicle(wp_path, startLaneID), isQueuing(false), distMovedInCurrSegment(0)
{

}

sim_mob::medium::MidVehicle::MidVehicle(std::vector<sim_mob::WayPoint> wp_path, int startLaneID, double length, double width)
: Vehicle(wp_path, startLaneID, length, width), isQueuing(false), distMovedInCurrSegment(0)
{

}

sim_mob::medium::MidVehicle::~MidVehicle(){}

double sim_mob::medium::MidVehicle::getDistanceMovedInSegment() const {
	return distMovedInCurrSegment;
}

void sim_mob::medium::MidVehicle::setDistanceMovedInSegment(double distance) {
	distMovedInCurrSegment += distance;
}
