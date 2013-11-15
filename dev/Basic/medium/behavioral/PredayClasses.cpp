//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayClasses.cpp
 *
 *  Created on: Nov 14, 2013
 *      Author: harish
 */

#pragma once
#include "PredayClasses.hpp"

#include <string>

using namespace std;
sim_mob::medium::TimeWindowAvailability::TimeWindowAvailability(double startTime, double endTime)
: startTime(startTime), endTime(endTime), availability(1) /*all time windows are available initially*/
{
	stringstream ss;
	ss << startTime << "," << endTime;
	windowString = ss.str();
}


