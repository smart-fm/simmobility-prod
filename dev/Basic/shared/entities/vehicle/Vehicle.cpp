//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Vehicle.hpp"

#include "geospatial/RoadSegment.hpp"
#include "geospatial/Node.hpp"

#include "logging/Log.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#endif

using namespace sim_mob;
using std::vector;

sim_mob::Vehicle::Vehicle(double lengthCM, double widthCM) :
	lengthCM(lengthCM), widthCM(widthCM), vehicleId(0), latMovement(0), fwdVelocity(0), latVelocity(0), fwdAccel(0), errorState(false), turningDirection(LCS_SAME), isQueuing(false), schedule(nullptr) {
}

sim_mob::Vehicle::Vehicle(int vehicleId, double lengthCM, double widthCM) :
	vehicleId(vehicleId), lengthCM(lengthCM), widthCM(widthCM), latMovement(0), fwdVelocity(0), latVelocity(0), fwdAccel(0), errorState(false), turningDirection(LCS_SAME), isQueuing(false),  schedule(nullptr)
{
}

sim_mob::Vehicle::Vehicle(const Vehicle& copyFrom) :
	lengthCM(copyFrom.lengthCM), widthCM(copyFrom.widthCM), vehicleId(copyFrom.vehicleId),
			latMovement(copyFrom.latMovement), fwdVelocity(copyFrom.fwdVelocity), latVelocity(copyFrom.latVelocity),
			fwdAccel(copyFrom.fwdAccel), posInIntersection(copyFrom.posInIntersection), errorState(
					copyFrom.errorState), turningDirection(LCS_SAME), isQueuing(copyFrom.isQueuing), schedule(copyFrom.schedule) {
}

void sim_mob::Vehicle::setPositionInIntersection(double x, double y) {
	posInIntersection.x = x;
	posInIntersection.y = y;
}

const DPoint& sim_mob::Vehicle::getPositionInIntersection()
{
	return posInIntersection;
}

const RoadSegment* sim_mob::Vehicle::getCurrSegment() const {
	return fwdMovement.getCurrSegment();
}

const RoadSegment* sim_mob::Vehicle::getNextSegment(bool inSameLink) const {
	return fwdMovement.getNextSegment(inSameLink);
}

const sim_mob::RoadSegment* sim_mob::Vehicle::getSecondSegmentAhead() {
	return fwdMovement.getNextToNextSegment();
}

const RoadSegment* sim_mob::Vehicle::hasNextSegment(bool inSameLink) const {
	if(!fwdMovement.isDoneWithEntireRoute()) {
		return fwdMovement.getNextSegment(inSameLink);
	}
	return nullptr;
}

const RoadSegment* sim_mob::Vehicle::getPrevSegment(bool inSameLink) const {
	return fwdMovement.getPrevSegment(inSameLink);
}

const Lane* sim_mob::Vehicle::getCurrLane() const {
	return fwdMovement.getCurrLane();
}

bool sim_mob::Vehicle::hasPath() const {
	return fwdMovement.isPathSet();
}

double sim_mob::Vehicle::getPositionInSegmentCM(){
	return fwdMovement.getPositionInSegmentCM();
}

void sim_mob::Vehicle::setPositionInSegmentCM(double newDistToEndCM){
	fwdMovement.setPositionInSegmentCM(newDistToEndCM);
}

double sim_mob::Vehicle::getVelocity() const {
	throw_if_error();
	return fwdVelocity;
}

double sim_mob::Vehicle::getLatVelocity() const {
	throw_if_error();
	return latVelocity;
}

double sim_mob::Vehicle::getAcceleration() const {
	throw_if_error();
	return fwdAccel;
}

LANE_CHANGE_SIDE sim_mob::Vehicle::getTurningDirection() const{
	return turningDirection;
}

void sim_mob::Vehicle::setVelocity(double value) {
	throw_if_error();
	fwdVelocity = value;
}

void sim_mob::Vehicle::setLatVelocity(double value) {
	throw_if_error();
	latVelocity = value;
}

void sim_mob::Vehicle::setAcceleration(double value) {
	throw_if_error();
	fwdAccel = value;
}

void sim_mob::Vehicle::setCurrPosition(DPoint currPosition)
{
	throw_if_error();
	currPos = currPosition;
}

const DPoint& sim_mob::Vehicle::getCurrPosition() const
{
	throw_if_error();
	return currPos;
}

void sim_mob::Vehicle::moveFwd_med(double amt) {
	throw_if_error();
	fwdMovement.advance_med(amt);
}

void sim_mob::Vehicle::actualMoveToNextSegmentAndUpdateDir_med() {
	throw_if_error();
	fwdMovement.actualMoveToNextSegmentAndUpdateDir_med();
}

void sim_mob::Vehicle::moveLat(double amt) {
	throw_if_error();
	latMovement += amt;
}

void sim_mob::Vehicle::setTurningDirection(LANE_CHANGE_SIDE direction)
{
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

bool sim_mob::Vehicle::isDone() const {
	throw_if_error();
	bool done = (fwdMovement.isDoneWithEntireRoute());
	return done;
}

#ifndef SIMMOB_DISABLE_MPI

#endif
