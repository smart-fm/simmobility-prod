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


TimeWindowAvailability::TimeWindowAvailability()
: startTime(0), endTime(0), availability(false)
{}

TimeWindowAvailability::TimeWindowAvailability(double startTime, double endTime, bool availability)
: startTime(startTime), endTime(endTime), availability(availability)
{
	if(startTime > endTime) { throw std::runtime_error("Invalid time window; start time cannot be greater than end time"); }
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

bool Tour::operator ==(const Tour& rhs) const {
	return (this == &rhs);
}

bool Tour::operator !=(const Tour& rhs) const {
	return !(*this == rhs); //call == operator overload
}

bool OD_Pair::operator ==(const OD_Pair& rhs) const
{
	return ((origin == rhs.origin) && (destination == rhs.destination));
}

bool OD_Pair::operator !=(const OD_Pair& rhs) const
{
	return !(*this == rhs);
}

bool OD_Pair::operator >(const OD_Pair& rhs) const
{
	if(origin > rhs.origin) { return true; }
	if(origin == rhs.origin && destination > rhs.destination) { return true; }
	return false;
}

bool OD_Pair::operator <(const OD_Pair& rhs) const
{
	if(origin < rhs.origin) { return true; }
	if(origin == rhs.origin && destination < rhs.destination) { return true; }
	return false;
}

}
}

