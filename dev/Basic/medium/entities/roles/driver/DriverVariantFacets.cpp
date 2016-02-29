//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverVariantFacets.hpp"

#include "entities/Person_MT.hpp"
#include "entities/Vehicle.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

namespace
{
/**
 * number of PCUs for driver variants
 */
const double BIKE_PCU = 0.5;
const double LIGHT_GOODS_VEHICLE_PCU = 1.0;
const double HEAVY_GOODS_VEHICLE_PCU = 2.0;

/**
 * length of vehicles of driver variants
 */
const double BIKE_LENGTH = BIKE_PCU * PASSENGER_CAR_UNIT; //m.
const double LGV_LENGTH = LIGHT_GOODS_VEHICLE_PCU * PASSENGER_CAR_UNIT; //m.
const double HGV_LENGTH = HEAVY_GOODS_VEHICLE_PCU * PASSENGER_CAR_UNIT; //m.
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

TruckerBehavior::TruckerBehavior() : DriverBehavior() {}
TruckerBehavior::~TruckerBehavior() {}

void TruckerBehavior::frame_init() {
	throw std::runtime_error("TruckerBehavior::frame_init is not implemented yet");
}

void TruckerBehavior::frame_tick() {
	throw std::runtime_error("TruckerBehavior::frame_tick is not implemented yet");
}

std::string TruckerBehavior::frame_tick_output() {
	throw std::runtime_error("TruckerBehavior::frame_tick_output is not implemented yet");
}

TruckerMovement::TruckerMovement() : DriverMovement() {}
TruckerMovement::~TruckerMovement() {}

void TruckerMovement::frame_init()
{
	bool pathInitialized = initializePath();
	if (pathInitialized)
	{
		Vehicle* newVeh = nullptr;
		switch(parentDriver->roleType)
		{
		case Role<Person_MT>::RL_TRUCKER_HGV:
		{
			newVeh = new Vehicle(Vehicle::HGV, HGV_LENGTH);
			break;
		}
		case Role<Person_MT>::RL_TRUCKER_LGV:
		{
			newVeh = new Vehicle(Vehicle::LGV, LGV_LENGTH);
			break;
		}
		default:
		{
			throw std::runtime_error("invalid roleType for Trucker");
		}
		}
		VehicleBase* oldVeh = parentDriver->getResource();
		safe_delete_item(oldVeh);
		parentDriver->setResource(newVeh);
	}
	else { parentDriver->getParent()->setToBeRemoved(); }
}
