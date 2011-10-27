/*
 * Vehicle.h
 *
 *  Created on: Oct 24, 2011
 *      Author: lzm
 */

#ifndef VEHICLE_H_
#define VEHICLE_H_

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
	double xVel;
	double yVel;
	double xAcc;
	double yAcc;

	int xPos_;
	int yPos_;
	double xVel_;
	double yVel_;
	double xAcc_;
	double yAcc_;
};

} /* namespace sim_mob */
#endif /* VEHICLE_H_ */
