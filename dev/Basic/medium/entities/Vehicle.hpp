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
	Vehicle(const VehicleType vehType, const double length, const double pcu)
	: sim_mob::VehicleBase(vehType, length, 0), PCU_Equivlent(pcu)
	{}

	const double getPcuEquivlent() const {
		return PCU_Equivlent;
	}

private:
	/**number of Passenger Car Units equivalent to this vehicle*/
	const double PCU_Equivlent;
};
} // end namespace medium
} // end namespace sim_mob
