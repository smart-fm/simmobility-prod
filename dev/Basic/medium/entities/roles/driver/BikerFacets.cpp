//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BikerFacets.hpp"

#include "entities/Person.hpp"
#include "entities/Vehicle.hpp"

namespace
{
/**
 * length of a bike is hard coded to 0.5 times the PCU for now.
 * TODO: this must be made configurable.
 */
const double BIKE_LENGTH = 0.5 * sim_mob::PASSENGER_CAR_UNIT; //cm. half of PASSENGER_CAR_UNIT
}

sim_mob::medium::BikerBehavior::BikerBehavior(sim_mob::Person* parentAgent) :
DriverBehavior(parentAgent), parentBiker(nullptr) {}

sim_mob::medium::BikerBehavior::~BikerBehavior() {}

void sim_mob::medium::BikerBehavior::frame_init() {
	throw std::runtime_error("BikerBehavior::frame_init is not implemented yet");
}

void sim_mob::medium::BikerBehavior::frame_tick() {
	throw std::runtime_error("BikerBehavior::frame_tick is not implemented yet");
}

void sim_mob::medium::BikerBehavior::frame_tick_output() {
	throw std::runtime_error("BikerBehavior::frame_tick_output is not implemented yet");
}

sim_mob::medium::BikerMovement::BikerMovement(sim_mob::Person* parentAgent) :
DriverMovement(parentAgent), parentBiker(nullptr) {}

sim_mob::medium::BikerMovement::~BikerMovement() {}

void sim_mob::medium::BikerMovement::frame_init()
{
	bool pathInitialized = initializePath();
	if (pathInitialized) {
		Vehicle* newVeh = new Vehicle(Vehicle::BIKE, BIKE_LENGTH);
		VehicleBase* oldBus = parentBiker->getResource();
		safe_delete_item(oldBus);
		parentBiker->setResource(newVeh);
	}
}
