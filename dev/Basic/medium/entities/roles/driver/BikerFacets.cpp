//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BikerFacets.hpp"

#include "entities/Person.hpp"
#include "entities/Vehicle.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

namespace
{
/**
 * number of PCUs for bike
 */
const double BIKE_PCU = 0.5;

/**
 * length of a bike is hard coded to 0.5 times the PCU for now.
 * TODO: this must be made configurable.
 */
const double BIKE_LENGTH = BIKE_PCU * PASSENGER_CAR_UNIT; //m. half of PASSENGER_CAR_UNIT

}

BikerBehavior::BikerBehavior() :
DriverBehavior(), parentBiker(nullptr) {}

BikerBehavior::~BikerBehavior() {}

void BikerBehavior::frame_init() {
	throw std::runtime_error("BikerBehavior::frame_init is not implemented yet");
}

void BikerBehavior::frame_tick() {
	throw std::runtime_error("BikerBehavior::frame_tick is not implemented yet");
}

std::string BikerBehavior::frame_tick_output() {
	throw std::runtime_error("BikerBehavior::frame_tick_output is not implemented yet");
}

BikerMovement::BikerMovement() : DriverMovement(), parentBiker(nullptr) {}

BikerMovement::~BikerMovement() {}

void BikerMovement::frame_init()
{
	bool pathInitialized = initializePath();
	if (pathInitialized)
	{
		Vehicle* newVeh = new Vehicle(Vehicle::BIKE, BIKE_LENGTH);
		VehicleBase* oldBike = parentBiker->getResource();
		safe_delete_item(oldBike);
		parentBiker->setResource(newVeh);
	}
	else { parentBiker->getParent()->setToBeRemoved(); }
}
