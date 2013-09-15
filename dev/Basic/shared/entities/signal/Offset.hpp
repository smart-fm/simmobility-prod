//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "defaults.hpp"

namespace sim_mob {

class Offset {
public:
	/*--------Offset----------*/
	Offset(){}
	void setnextOffset(double nextCL);
	double getcurrOffset()const {return currOffset;}
	double getnextOffset();
	void updateCurrOffset();
	void update(double nextCL);

	//parameters for calculating next cycle length
	const static double DSmax;
	const static double DSmed;
	const static double DSmin;

	const static double CLmax;
	const static double CLmed;
	const static double CLmin;

	//parameters for calculating next Offset
	const static double CL_low;
	const static double CL_up;
	const static double Off_low;
	const static double Off_up;

	const static double fixedCL;

private:
	/*-------------------------------------------------------------------------
	 * -------------------Offset Indicators------------------------------
	 * ------------------------------------------------------------------------*/
	//current and next Offset
	double currOffset, nextOffset;
};

}//namespace sim_mob
