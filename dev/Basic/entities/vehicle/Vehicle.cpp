/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Vehicle.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: lzm
 */

#include "Vehicle.hpp"

using namespace sim_mob;

sim_mob::Vehicle::Vehicle() {
	//assume that all the car has the same size
	length=400;
	width=200;
}

sim_mob::Vehicle::~Vehicle() {
	// TODO Auto-generated destructor stub
}
