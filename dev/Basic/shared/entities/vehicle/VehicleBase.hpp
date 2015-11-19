//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "geospatial/network/RoadSegment.hpp"

namespace sim_mob {
/**
 * Passenger car units.
 * TODO: Defining this as a global variable for now. Must make this
 * configurable in future.
 */
const double PASSENGER_CAR_UNIT = 4.0; //m.

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
		BIKE,
		OTHER
	};

	VehicleBase(const VehicleType vehType, const double length, const double width)
	: vehicleType(vehType), lengthM(length), widthM(width), moving(true)
	{}

	VehicleBase(const VehicleBase& copy)
	: vehicleType(copy.vehicleType), lengthM(copy.lengthM),
	  widthM(copy.widthM), moving(copy.moving)
	{}

	virtual ~VehicleBase() {}

	const double getLengthInM() const {
		return lengthM;
	}

	const double getWidthInM() const {
		return widthM;
	}

	const VehicleType getVehicleType() const {
		return vehicleType;
	}

	bool isMoving() const {
		return moving;
	}

	void setMoving(bool moving) {
		this->moving = moving;
	}

	/*virtual std::vector<const sim_mob::RoadSegment *>::iterator getPathIterator()
	{
		throw std::runtime_error("VehicleBase::getPathIterator is not implemented");
	}

	virtual std::vector<const sim_mob::RoadSegment *>::iterator getPathIteratorEnd()
	{
		throw std::runtime_error("VehicleBase::getPathIteratorEnd is not implemented");
	}

	virtual const sim_mob::RoadSegment * getCurrSegment() const
	{
		throw std::runtime_error("VehicleBase::getCurrSegment is not implemented");
	}

	virtual double getAcceleration() const
	{
		throw std::runtime_error("VehicleBase::getAcceleration is not implemented");
	}

	virtual double getVelocity() const
	{
		throw std::runtime_error("VehicleBase::getVelocity is not implemented");
	}*/

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

}// end namespace sim_mob
