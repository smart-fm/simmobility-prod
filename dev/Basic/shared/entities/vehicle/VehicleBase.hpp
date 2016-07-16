//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob
{
/**
 * Passenger car units.
 * TODO: Defining this as a global variable for now. Must make this
 * configurable in future.
 */
const double PASSENGER_CAR_UNIT = 4.0; //m.

/**
 * number of PCU for bus
 */
const double BUS_PCU = 2;
/**
 * length of a bus is hard coded to 3 times the PCU for now.
 * TODO: this must be made configurable.
 */
const double BUS_LENGTH = sim_mob::BUS_PCU * sim_mob::PASSENGER_CAR_UNIT; // 2 times PASSENGER_CAR_UNIT

/**
 * A simple base class for all Vehicles
 *
 * \author Harish Loganathan
 */
class VehicleBase
{
public:
	enum VehicleType
	{
		CAR, BUS, TAXI, BIKE, LGV, HGV, OTHER
	};

	VehicleBase(const VehicleType vehType, const double length, const double width) :
			vehicleType(vehType), lengthM(length), widthM(width), moving(true)
	{
	}

	VehicleBase(const VehicleBase& copy) :
			vehicleType(copy.vehicleType), lengthM(copy.lengthM), widthM(copy.widthM), moving(copy.moving)
	{
	}

	virtual ~VehicleBase()
	{
	}

	const double getLengthInM() const
	{
		return lengthM;
	}

	const double getWidthInM() const
	{
		return widthM;
	}

	const VehicleType getVehicleType() const
	{
		return vehicleType;
	}

	bool isMoving() const
	{
		return moving;
	}

	void setMoving(bool moving)
	{
		this->moving = moving;
	}

protected:
	/**length of the vehicle in m*/
	const double lengthM;
	/**width of the vehicle in m*/
	const double widthM;
	/**type of vehicle*/
	const VehicleType vehicleType;
	/**flag to indicate moving status of vehicle*/
	bool moving;
};

} // end namespace sim_mob
