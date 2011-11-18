/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Vehicle.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: lzm
 */

#include "Vehicle.hpp"

using namespace sim_mob;


sim_mob::Vehicle::Vehicle() : length(400), width(200), error_state(true)
{
}

double sim_mob::Vehicle::getX() const
{
	throw_if_error();
	return fwdMovement.getPosition().x;
}

double sim_mob::Vehicle::getY() const
{
	throw_if_error();
	return fwdMovement.getPosition().y;
}

/*double sim_mob::Vehicle::getDistanceMovedInSegment() const
{
	throw_if_error();
	return position.getAmountMoved();
}*/

double sim_mob::Vehicle::getVelocity() const
{
	throw_if_error();
	return fwdVelocity;
}

double sim_mob::Vehicle::getLatVelocity() const
{
	throw_if_error();
	return latVelocity;
}

double sim_mob::Vehicle::getAcceleration() const
{
	throw_if_error();
	return fwdAccel;
}

/*bool sim_mob::Vehicle::reachedSegmentEnd() const
{
	throw_if_error();
	return position.reachedEnd();
}*/

double sim_mob::Vehicle::getAngle() const
{
	throw_if_error();
	DynamicVector temp(
		fwdMovement.getCurrPolypoint().getX(), fwdMovement.getCurrPolypoint().getY(),
		fwdMovement.getNextPolypoint().getX(), fwdMovement.getNextPolypoint().getY()
	);
	return temp.getAngle();
}

void sim_mob::Vehicle::setVelocity(double value)
{
	throw_if_error();
	fwdVelocity = value;
}

void sim_mob::Vehicle::setLatVelocity(double value)
{
	throw_if_error();
	latVelocity = value;
}

void sim_mob::Vehicle::setAcceleration(double value)
{
	throw_if_error();
	fwdAccel = value;
}

void sim_mob::Vehicle::moveFwd(double amt)
{
	throw_if_error();
	fwdMovement.advance(amt);
}

void sim_mob::Vehicle::moveLat(double amt)
{
	throw_if_error();
	latMovement += amt;
}

void sim_mob::Vehicle::resetLateralMovement()
{
	throw_if_error();
	latMovement = 0;
}

double sim_mob::Vehicle::getLateralMovement() const
{
	throw_if_error();
	return latMovement;
}

bool sim_mob::Vehicle::isInIntersection() const
{
	throw_if_error();
	return fwdMovement.isInIntersection();
}


void sim_mob::Vehicle::moveToNextSegmentAfterIntersection()
{
	throw_if_error();
	fwdMovement.leaveIntersection();
}

bool sim_mob::Vehicle::isDone() const
{
	throw_if_error();
	return fwdMovement.isDoneWithEntireRoute();
}

/*void sim_mob::Vehicle::newPolyline(sim_mob::Point2D firstPoint, sim_mob::Point2D secondPoint)
{
	//Get a generic vector pointing in the magnitude of the current polyline
	DynamicVector currDir(0, 0,
		secondPoint.getX() - firstPoint.getX(),
		secondPoint.getY() - firstPoint.getY()
	);

	//Double check that we're not doing something silly.
	error_state = currDir.getMagnitude()==0;
	throw_if_error();

	//Sync velocity.
	repositionAndScale(velocity, currDir);
	repositionAndScale(velocity_lat, currDir);
	velocity_lat.flipLeft();

	//Sync acceleration
	repositionAndScale(accel, currDir);

	//Sync position
	position.moveToNewVect(DynamicVector(
		firstPoint.getX(), firstPoint.getY(),
		secondPoint.getX(), secondPoint.getY()
	));
}*/

/*const DynamicVector& TEMP_retrieveFwdVelocityVector() {
	throw_if_error();
	return velocity;
}*/


