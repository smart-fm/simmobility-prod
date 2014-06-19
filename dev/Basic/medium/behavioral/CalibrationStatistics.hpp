//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <cstdlib>
#include <vector>

namespace sim_mob
{
namespace medium
{
class CalibrationStatistics
{
private:
	//city level number of tours and trips
	/**
	 * vector to track the number of persons with different number of tours.
	 * Spoken semantics apply.
	 * e.g. numPersonsWithTourCount[3] is the number of persons who have 3 tours.
	 */
	std::vector<unsigned> numPersonsWithTourCount;

	/**
	 * vector to track the number of persons making different number of stops
	 * Spoken semantics apply.
	 * e.g. numPersonsWithStopCount[5] is the number of persons who make 5 stops all together
	 */
	std::vector<unsigned> numPersonsWithStopCount;

	/**
	 * mode share statistics
	 * Lookup:
	 * -- 0 for public bus; 1 for MRT/LRT; 2 for private bus; 3 for drive1;
	 * -- 4 for shared2; 5 for shared3+; 6 for motor; 7 for walk; 8 for taxi
	 *
	 * e.g. numTripsWithMode[5] is the number of trips with shared 2 as mode
	 */
	std::vector<unsigned> numTripsWithMode;

	/**
	 * trip distance statistics
	 * Lookup:
	 * -- 0 [0,5) km; 1 [5,10) km; 2 [10,15) km; 3 [15,20) km; 4 [20,25) km; 5 [25,30) km;
	 * -- 6 [30,40) km; 7 [40,infinity) km;
	 *
	 * e.g. numTripsWithDistance[1] is the number of trips with travel distance in range [5,10)
	 */
	std::vector<unsigned> numTripsWithDistance;

public:
	CalibrationStatistics();
	virtual ~CalibrationStatistics();

	/**
	 * concatenates all statistics into outStatistics
	 * @param outStatistics vector to be populated with all statistics
	 */
	void getAllStatistics(std::vector<unsigned>& outStatistics);

	/**
	 * adds 1 to the current number of persons with 'numTours' tours
	 * @param numTours number of tours to add to
	 * @return true if addition was successful; false otherwise
	 */
	bool addPersonToTourCountStats(size_t numTours);

	/**
	 * adds 1 to the current number of persons with 'numStops' stops
	 * @param numStops number of stops to add to
	 * @return true if addition was successful; false otherwise
	 */
	bool addPersonToStopCountStats(size_t numStops);

	/**
	 * adds 1 to the current number of trips with 'mode' as mode.
	 * Expected modes: [1,9] as used by the preday models
	 * -- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
	 * -- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
	 *
	 * @param mode the mode in [0,8] to add to
	 * @return true if addition was successful; false otherwise
	 */
	bool addTripToModeShareStats(size_t mode);

	/**
	 * adds 1 to the current number of trips with 'mode' as mode.
	 * @param mode the mode to add to
	 * @return true if addition was successful; false otherwise
	 */
	bool addTripToTravelDistanceStats(double distance);
};
}
}

