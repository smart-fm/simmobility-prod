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
	double fraction = startTime - std::floor(startTime);
	if((fraction != 0.25 && fraction != 0.75) || startTime < 3.25 || startTime > 26.75) {
		throw std::runtime_error("Invalid start time");
	}
	fraction = endTime - std::floor(endTime);
	if((fraction != 0.25 && fraction != 0.75) || endTime < 3.25 || endTime > 26.75) {
		throw std::runtime_error("Invalid end time");
	}
}

namespace sim_mob {
namespace medium {
/**
 * initializes all possible time windows in a day and returns a vector of all windows.
 */
std::vector<TimeWindowAvailability> getAllTimeWindows() {
	// Following values are hard coded for now.
	double dayStart = 3.25;
	double dayEnd = 26.75;
	double stepSz = 0.5;
	size_t numTimeWindows = 1176;

    std::vector<TimeWindowAvailability> timeWindows;
    timeWindows.reserve(numTimeWindows); // to avoid unnecessary calls to copy constructor
    for(double i = dayStart; i <= dayEnd; i=i+stepSz) {
        for(double j = i; j <= dayEnd; j=j+stepSz) {
        	timeWindows.push_back(TimeWindowAvailability(i,j));
        }
    }
    return timeWindows;
}

const std::vector<TimeWindowAvailability> TimeWindowAvailability::timeWindowsLookup = getAllTimeWindows();
}
}


