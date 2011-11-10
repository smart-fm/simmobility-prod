/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Vehicle.h
 *
 *  Created on: Oct 24, 2011
 *      Author: lzm
 */

#pragma once

#include "util/RelAbsPoint.hpp"


namespace sim_mob {

class Vehicle {
public:
	Vehicle();
	virtual ~Vehicle();

public:
	double length;				//length of the vehicle
	double width;				//width of the vehicle
	double timeStep;			//time step size of simulation

	int xPos;
	int yPos;
	int xPos_;
	int yPos_;

	//Test2: This is really more of a vector...
	DynamicVector velocity;
	DynamicVector velocity_lat; //Lateral velocity. Positive means pointing left.
	//double xVel;
	//double yVel;
	//double xVel_;
	//double yVel_;

	//Test2: This is really more of a vector...
	//RelAbsPoint accel;
	DynamicVector accel;
	//double xAcc;
	//double yAcc;
	//double xAcc_;
	//double yAcc_;
};

} // namespace sim_mob
