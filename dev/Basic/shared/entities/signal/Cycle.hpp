//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "defaults.hpp"

namespace sim_mob {

class Cycle {

private:
	//previous,current and next cycle length
	double prevCL,currCL,nextCL;

	//two previous RL for calculating the current RL0
	double prevRL1,prevRL2;

public:
	//get the parameters in SCATS
	double getprevCL();
	double getcurrCL();
	double getnextCL();
	double getpreRL1();
	double getpreRL2();
	double setnextCL(double DS);
	void Update(double DS);
	void updateprevCL();
	void updatecurrCL();
	void setCurrCL(double length) { currCL = length; }//used for initial feed
};



}
;//namspace
