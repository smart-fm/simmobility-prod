//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "CalibrationStatistics.hpp"

namespace
{
const size_t SIZE_NUM_TOURS_STATS = 4; // num. of tours for a person is in [0,3]
const size_t SIZE_NUM_STOPS_STATS = 22; //num. of stops per person is in [0,21] (3 tours * (3 stops in 1st half + 3 stops in 2nd half + 1 primary stop))
const size_t SIZE_MODE_SHARE_STATS = 9; // there are 9 modes
const size_t SIZE_TRAVEL_DISTANCE_STATS = 8; // there are 8 classes of travel distances
}

sim_mob::medium::CalibrationStatistics::CalibrationStatistics()
: numPersonsWithTourCount(std::vector<unsigned>(SIZE_NUM_TOURS_STATS,0)),
  numPersonsWithStopCount(std::vector<unsigned>(SIZE_NUM_STOPS_STATS,0)),
  numTripsWithMode(std::vector<unsigned>(SIZE_MODE_SHARE_STATS,0)),
  numTripsWithDistance(std::vector<unsigned>(SIZE_TRAVEL_DISTANCE_STATS,0))
{
}

sim_mob::medium::CalibrationStatistics::~CalibrationStatistics()
{
}

void sim_mob::medium::CalibrationStatistics::getAllStatistics(std::vector<unsigned>& outStatistics)
{
	outStatistics.insert(outStatistics.end(), numPersonsWithTourCount.begin(), numPersonsWithTourCount.end());
	outStatistics.insert(outStatistics.end(), numPersonsWithStopCount.begin(), numPersonsWithStopCount.end());
	outStatistics.insert(outStatistics.end(), numTripsWithMode.begin(), numTripsWithMode.end());
	outStatistics.insert(outStatistics.end(), numTripsWithDistance.begin(), numTripsWithDistance.end());
}

bool sim_mob::medium::CalibrationStatistics::addPersonToTourCountStats(size_t numTours)
{
	if(numTours < numPersonsWithTourCount.size())
	{
		numPersonsWithTourCount[numTours] = numPersonsWithTourCount[numTours] + 1;
		return true;
	}
	return false;
}

bool sim_mob::medium::CalibrationStatistics::addPersonToStopCountStats(size_t numStops)
{
	if(numStops < numPersonsWithStopCount.size())
	{
		numPersonsWithStopCount[numStops] = numPersonsWithStopCount[numStops] + 1;
		return true;
	}
	return false;
}

bool sim_mob::medium::CalibrationStatistics::addTripToModeShareStats(size_t mode)
{
	if(mode > 9 || mode < 1) { return false; }
	numTripsWithMode[mode-1] = numTripsWithMode[mode-1] + 1;
	return true;
}

bool sim_mob::medium::CalibrationStatistics::addTripToTravelDistanceStats(double distance)
{
	if(distance < 0) { return false; } // inadmissible
	else if(distance >= 40) { numTripsWithDistance[7] = numTripsWithDistance[7] + 1; } // [40,infinity) km
	else if(distance >= 30) { numTripsWithDistance[6] = numTripsWithDistance[6] + 1; } // [30, 40) km
	else if(distance >= 25) { numTripsWithDistance[5] = numTripsWithDistance[5] + 1; } // [25, 30) km
	else if(distance >= 20) { numTripsWithDistance[4] = numTripsWithDistance[4] + 1; } // [20, 25) km
	else if(distance >= 15) { numTripsWithDistance[3] = numTripsWithDistance[3] + 1; } // [15, 20) km
	else if(distance >= 10) { numTripsWithDistance[2] = numTripsWithDistance[2] + 1; } // [10, 15) km
	else if(distance >= 5) { numTripsWithDistance[1] = numTripsWithDistance[1] + 1; }  // [5 , 10) km
	else { numTripsWithDistance[0] = numTripsWithDistance[0] + 1; }                    // [0 , 5 ) km
	return true;
}
