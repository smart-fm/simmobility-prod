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
	double timeStep;			//time step size of simulation

	//Place at a given location.
	void placeAt(Point2D pos, Point2D target) {
		position = MovementVector(DynamicVector(pos.getX(), pos.getY(), target.getX(), target.getY()));
	}

	//Accessors
	double getX() const {
		return position.getX();
	}
	double getY() const {
		return position.getY();
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







private:
	MovementVector position;
	DynamicVector velocity;
	DynamicVector velocity_lat; //Lateral velocity. Positive means pointing left.
	DynamicVector accel;
};

} // namespace sim_mob
