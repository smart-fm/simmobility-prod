/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Vehicle.h
 *
 *  Created on: Oct 24, 2011
 *      Author: lzm
 */

#pragma once

#include <stdexcept>

#include "util/MovementVector.hpp"
#include "util/DynamicVector.hpp"


namespace sim_mob {

class Vehicle {
public:
	Vehicle() : length(400), width(200), error_state(true) {
	}

public:
	double length;				//length of the vehicle
	double width;				//width of the vehicle
	//double timeStep;			//time step size of simulation

	//Place at a given location.
	/*void placeAt(Point2D pos, Point2D target) {
		position = MovementVector(DynamicVector(pos.getX(), pos.getY(), target.getX(), target.getY()));
	}*/

	//Accessors
	double getX() const {
		throw_if_error();
		return position.getX();
	}
	double getY() const {
		throw_if_error();
		return position.getY();
	}
	double getDistanceMovedInSegment() const {
		throw_if_error();
		return position.getAmountMoved();
	}
	double getVelocity() const {
		throw_if_error();
		return velocity.getMagnitude();
	}
	double getLatVelocity() const {
		throw_if_error();
		return velocity_lat.getMagnitude();
	}
	double getAcceleration() const {
		throw_if_error();
		return accel.getMagnitude();
	}
	bool reachedSegmentEnd() const {
		throw_if_error();
		return position.reachedEnd();
	}

	//Special
	double getAngleBasedOnVelocity() const {
		throw_if_error();
		return velocity.getAngle();
	}

	//Modifiers
	void setVelocity(double value) {
		throw_if_error();
		velocity.scaleVectTo(value);
	}
	void setLatVelocity(double value) {
		throw_if_error();
		velocity_lat.scaleVectTo(value);
	}
	void setAcceleration(double value) {
		throw_if_error();
		accel.scaleVectTo(value);
	}
	void moveFwd(double amt) {
		throw_if_error();
		position.moveFwd(amt);
	}
	void moveLat(double amt) {
		throw_if_error();
		position.moveLat(amt);
	}
	void resetLateralMovement() {
		throw_if_error();
		position.resetLateral();
	}
	double getLateralMovement() {
		throw_if_error();
		return position.getLateralMovement();
	}

	//Complex
	void newPolyline(sim_mob::Point2D firstPoint, sim_mob::Point2D secondPoint) {
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
			firstPoint.getX(), secondPoint.getY(),
			firstPoint.getX(), secondPoint.getY()
		));
	}

	//Temporary; workaround
	const DynamicVector& TEMP_retrieveFwdVelocityVector() {
		throw_if_error();
		return velocity;
	}


private:
	MovementVector position;
	DynamicVector velocity;
	DynamicVector velocity_lat; //Lateral velocity. Positive means pointing left.
	DynamicVector accel;

	//NOTE: The error state is a temporary sanity check to help me debug this class. There are certainly
	//      better ways to handle this (e.g., non-default constructor).
	bool error_state;
	void throw_if_error() const {
		if (error_state) {
			throw std::runtime_error("Error: can't perform certain actions on an uninitialized vehicle.");
		}
	}

	//Helper function: Reposition and take care of any remaining velocity.
	void repositionAndScale(DynamicVector& item, const DynamicVector& newHeading) {
		double oldMag = item.getMagnitude();
		item = newHeading;
		item.scaleVectTo(oldMag);
	}
};

} // namespace sim_mob
