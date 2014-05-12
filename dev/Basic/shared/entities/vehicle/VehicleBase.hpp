//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob {
/**
 * Passenger car units.
 * TODO: Defining this as a global variable for now. Must make this
 * configurable in future.
 */
const double PASSENGER_CAR_UNIT = 400.0; //cm; 4 m.

/**
 * A simple base class for all Vehicles
 *
 * \author Harish Loganathan
 */
class VehicleBase {
public:
	enum VehicleType {
		CAR,
		BUS,
		OTHER
	};

	VehicleBase(const VehicleType vehType, const double length, const double width)
	: vehicleType(vehType), lengthCM(length), widthCM(width)
	{}

	VehicleBase(const VehicleBase& copy)
	: vehicleType(copy.vehicleType), lengthCM(copy.lengthCM),
	  widthCM(copy.widthCM)
	{}

	virtual ~VehicleBase() {}

	const double getLengthCm() const {
		return lengthCM;
	}

	const double getWidthCm() const {
		return widthCM;
	}

	const VehicleType getVehicleType() const {
		return vehicleType;
	}

protected:
	/**length of the vehicle in cm*/
	const double lengthCM;
	/**width of the vehicle in cm*/
	const double widthCM;
	/**type of vehicle*/
	const VehicleType vehicleType;
};

}// end namespace sim_mob
