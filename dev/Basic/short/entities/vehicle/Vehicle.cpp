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

sim_mob::Vehicle::Vehicle(const VehicleType vehType, double lengthM, double widthM) :
	VehicleBase(vehType,lengthM,widthM), vehicleId(0), latMovement(0),
	forwardVelocity(0), lateralVelocity(0), forwardAcceleration(0), errorState(false),
	turningDirection(LANE_CHANGE_TO_NONE)
{}

sim_mob::Vehicle::Vehicle(const VehicleType vehType, int vehicleId, double lengthM, double widthM) :
		VehicleBase(vehType,lengthM,widthM), vehicleId(vehicleId),
		latMovement(0), forwardVelocity(0), lateralVelocity(0), forwardAcceleration(0),
		errorState(false), turningDirection(LANE_CHANGE_TO_NONE)
{}

sim_mob::Vehicle::Vehicle(const Vehicle& copyFrom) :
	VehicleBase(copyFrom), vehicleId(copyFrom.vehicleId), latMovement(copyFrom.latMovement),
	forwardVelocity(copyFrom.forwardVelocity), lateralVelocity(copyFrom.lateralVelocity), forwardAcceleration(copyFrom.forwardAcceleration),
	posInIntersection(copyFrom.posInIntersection), errorState(copyFrom.errorState),
	turningDirection(LANE_CHANGE_TO_NONE){
}

void sim_mob::Vehicle::setPositionInIntersection(double x, double y) {
	posInIntersection.setX(x);
	posInIntersection.setY(y);
}

const Point& sim_mob::Vehicle::getPositionInIntersection()
{
	return posInIntersection;
}

double sim_mob::Vehicle::getVelocity() const {
	throw_if_error();
	return forwardVelocity;
}

double sim_mob::Vehicle::getLateralVelocity() const {
	throw_if_error();
	return lateralVelocity;
}

double sim_mob::Vehicle::getAcceleration() const {
	throw_if_error();
	return forwardAcceleration;
}

LaneChangeTo sim_mob::Vehicle::getTurningDirection() const{
	return turningDirection;
}

void sim_mob::Vehicle::setVelocity(double value) {
	throw_if_error();
	forwardVelocity = value;
}

void sim_mob::Vehicle::setLateralVelocity(double value) {
	throw_if_error();
	lateralVelocity = value;
}

void sim_mob::Vehicle::setAcceleration(double value) {
	throw_if_error();
	forwardAcceleration = value;
}

void sim_mob::Vehicle::setCurrPosition(Point currPosition) {
	throw_if_error();
	currPos = currPosition;
}

const Point& sim_mob::Vehicle::getCurrPosition() const {
	throw_if_error();
	return currPos;
}

void sim_mob::Vehicle::moveLat(double amt) {
	throw_if_error();
	latMovement += amt;
}

void sim_mob::Vehicle::setTurningDirection(LaneChangeTo direction) {
	throw_if_error();
	turningDirection = direction;
}

void sim_mob::Vehicle::resetLateralMovement() {
	throw_if_error();
	latMovement = 0;
}

double sim_mob::Vehicle::getLateralMovement() const {
	throw_if_error();
	return latMovement;
}

#ifndef SIMMOB_DISABLE_MPI

#endif
