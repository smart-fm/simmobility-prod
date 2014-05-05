//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file Vehicle.hpp
 *
 * \author Harish Loganathan
 */

#pragma once

namespace sim_mob {
namespace medium {

/**
 * representation of mid-term vehicle
 */
class Vehicle {
public:
	enum VehicleType {
		CAR,
		BUS,
		OTHER
	};

	Vehicle(const VehicleType vehType, const double length, const double pcu) :
		vehicleType(vehType), length(length), PCU_Equivlent(pcu)
	{}

	const double getLength() const {
		return length;
	}

	const double getPcuEquivlent() const {
		return PCU_Equivlent;
	}

	const VehicleType getVehicleType() const {
		return vehicleType;
	}


private:
	/**length of the vehicle*/
	const double length;
	/**number of Passenger Car Units equivalent to this vehicle*/
	const double PCU_Equivlent;
	/**type of vehicle*/
	const VehicleType vehicleType;
};
} // end namespace medium
} // end namespace sim_mob
