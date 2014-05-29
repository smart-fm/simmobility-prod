//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file Vehicle.hpp
 *
 * \author Harish Loganathan
 */

#pragma once
#include "entities/vehicle/VehicleBase.hpp"

namespace sim_mob {
namespace medium {

/**
 * representation of mid-term vehicle
 */
class Vehicle : public sim_mob::VehicleBase {
public:
	Vehicle(const VehicleType vehType, const double length)
	: sim_mob::VehicleBase(vehType, length, 0),
	  passengerCarUnits(length/PASSENGER_CAR_UNIT)
	{}

	const double getPassengerCarUnits() const {
		return passengerCarUnits;
	}

private:
	/**
	 * number of Passenger Car Units equivalent to this vehicle
	 * we assume 1 PCU = 400cm (following DynaMIT).
	 */
	const double passengerCarUnits;
};
} // end namespace medium
} // end namespace sim_mob
