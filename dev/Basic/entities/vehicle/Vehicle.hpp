/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Vehicle.h
 *
 *  Created on: Oct 24, 2011
 *      Author: lzm
 */

#pragma once

#include "util/MovementVector.hpp"
#include "util/DynamicVector.hpp"


namespace sim_mob {

class Vehicle {
public:
	Vehicle() : length(400), width(200) {
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
		return position.getX();
	}
	double getY() const {
		return position.getY();
	}
	double getDistanceMovedInSegment() const {
		return position.getAmountMoved();
	}
	double getVelocity() const {
		return velocity.getMagnitude();
	}
	double getLatVelocity() const {
		return velocity_lat.getMagnitude();
	}
	double getAcceleration() const {
		return accel.getMagnitude();
	}
	bool reachedSegmentEnd() const {
		return position.reachedEnd();
	}

	//Special
	double getAngleBasedOnVelocity() const {
		return velocity.getAngle();
	}

	//Modifiers
	void setVelocity(double value) {
		velocity.scaleVectTo(value);
	}
	void setLatVelocity(double value) {
		velocity_lat.scaleVectTo(value);
	}
	void setAcceleration(double value) {
		accel.scaleVectTo(value);
	}
	void moveFwd(double amt) {
		position.moveFwd(amt);
	}
	void moveLat(double amt) {
		position.moveLat(amt);
	}
	void resetLateralMovement() {
		position.resetLateral();
	}
	double getLateralMovement() {
		return position.getLateralMovement();
	}

	//Complex
	void newPolyline(sim_mob::Point2D firstPoint, sim_mob::Point2D secondPoint) {
		//Get a generic vector pointing in the magnitude of the current polyline
		DynamicVector currDir(0, 0,
			secondPoint.getX() - firstPoint.getX(),
			secondPoint.getY() - firstPoint.getY()
		);

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
		return velocity;
	}


private:
	MovementVector position;
	DynamicVector velocity;
	DynamicVector velocity_lat; //Lateral velocity. Positive means pointing left.
	DynamicVector accel;

	//Helper function: Reposition and take care of any remaining velocity.
	void repositionAndScale(DynamicVector& item, const DynamicVector& newHeading) {
		double oldMag = item.getMagnitude();
		item = newHeading;
		item.scaleVectTo(oldMag);
	}
};

} // namespace sim_mob
