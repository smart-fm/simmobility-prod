//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayClasses.cpp
 *
 *  Created on: Nov 14, 2013
 *      Author: harish
 */

#include "PredayClasses.hpp"

#include <string>

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;

TimeWindowAvailability::TimeWindowAvailability(double startTime, double endTime)
: startTime(startTime), endTime(endTime), availability(1) /*all time windows are available initially*/
{
	if(startTime > endTime) {
		throw std::runtime_error("Invalid time window; start time cannot be greater than end time");
	}
}

namespace sim_mob {
namespace medium {
/**
 * initializes all possible time windows in a day and returns a vector of all windows.
 */
std::vector<TimeWindowAvailability> insertAllTimeWindows() {
	// Following values are hard coded for now.
	double dayStart = 1;
	double dayEnd = 48;
	double stepSz = 1;
	size_t numTimeWindows = 1176;

    std::vector<TimeWindowAvailability> timeWindows;
    for(double i = dayStart; i <= dayEnd; i=i+stepSz) {
        for(double j = i; j <= dayEnd; j=j+stepSz) {
        	timeWindows.push_back(TimeWindowAvailability(i,j));
        }
    }
    return timeWindows;
}

const std::vector<TimeWindowAvailability> TimeWindowAvailability::timeWindowsLookup = insertAllTimeWindows();
}
}


