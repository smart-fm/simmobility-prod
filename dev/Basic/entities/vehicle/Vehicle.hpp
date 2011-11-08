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
	double xAcc;
	double yAcc;

	int xPos_;
	int yPos_;
	double xAcc_;
	double yAcc_;

	//Testing moving these into their own class:
	RelAbsPoint velocity;
	//double xVel;
	//double yVel;
	//double xVel_;
	//double yVel_;
};

} // namespace sim_mob
