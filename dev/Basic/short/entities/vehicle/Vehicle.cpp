//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Vehicle.hpp"

#include "logging/Log.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/RoadSegment.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

using namespace sim_mob;
using std::vector;

Vehicle::Vehicle(const VehicleType vehType, double lengthM, double widthM, const std::string& vehName) :
VehicleBase(vehType, lengthM, widthM), vehicleId(0), latMovement(0), vehicleName(vehName), forwardVelocity(0), lateralVelocity(0), 
forwardAcceleration(0), turningDirection(LANE_CHANGE_TO_NONE)
{
}

Vehicle::Vehicle(const VehicleType vehType, int vehicleId, double lengthM, double widthM, const std::string& vehName) :
VehicleBase(vehType, lengthM, widthM), vehicleId(vehicleId), vehicleName(vehName), latMovement(0), forwardVelocity(0), 
lateralVelocity(0), forwardAcceleration(0), turningDirection(LANE_CHANGE_TO_NONE)
{
}

Vehicle::~Vehicle()
{
}

double Vehicle::getVelocity() const
{
	return forwardVelocity;
}

double Vehicle::getLateralVelocity() const
{
	return lateralVelocity;
}

double Vehicle::getAcceleration() const
{
	return forwardAcceleration;
}

LaneChangeTo Vehicle::getTurningDirection() const
{
	return turningDirection;
}

void Vehicle::setVelocity(double value)
{
	forwardVelocity = value;
}

void Vehicle::setLateralVelocity(double value)
{
	lateralVelocity = value;
}

void Vehicle::setAcceleration(double value)
{
	forwardAcceleration = value;
}

void Vehicle::setCurrPosition(Point currPosition)
{
	currPos = currPosition;
}

const Point& Vehicle::getCurrPosition() const
{
	return currPos;
}

void Vehicle::moveLat(double amt)
{
	latMovement += amt;
}

void Vehicle::setTurningDirection(LaneChangeTo direction)
{
	turningDirection = direction;
}

void Vehicle::resetLateralMovement()
{
	latMovement = 0;
}

const std::string& Vehicle::getVehicleName() const
{
	return vehicleName;
}

double Vehicle::getLateralMovement() const
{
	return latMovement;
}

