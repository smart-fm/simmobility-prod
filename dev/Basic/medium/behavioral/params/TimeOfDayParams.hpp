//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TimeOfDayParams.hpp
 *
 *  Created on: Nov 21, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include <vector>

namespace sim_mob {
namespace medium {

/**
 * Simple class to store information pertaining time of day models (tour and stop)
 * \note This class is used by the mid-term behavior models.
 *
 * \author Harish Loganathan
 */
class TimeOfDayParams {
public:
	TimeOfDayParams() : numTimeWindows(48) {}
	virtual ~TimeOfDayParams() {}

	/**
	 * Templated function to get a specific element from first half tour vector travelTimesFirstHalfTour
	 *
	 * @returns the value from the vector if index < 48; returns -1 otherwise;
	 * \note -1 is an invalid value for travel time and the caller must check for this value.
	 */
	template <unsigned index>
	int getTT_FirstHalfTour(){
		if(index < numTimeWindows) {
			return travelTimesFirstHalfTour[index];
		}
		return -1;
	}

	/**
	 * Templated function to get a specific element from second half tour vector travelTimesSecondHalfTour
	 *
	 * @returns the value from the vector if index < 48; returns -1 otherwise;
	 * \note -1 is an invalid value for travel time and the caller must check for this value.
	 */
	template <unsigned index>
	int getTT_SecondHalfTour(){
		if(index < numTimeWindows) {
			return travelTimesSecondHalfTour[index];
		}
		return -1;
	}

	/**
	 * Vector storing the travel times for first and second half-tours in various all half-hour windows within a day.
	 * The day starts at 0300Hrs and ends at 2659Hrs.
	 * The half-hour windows are 0300-0330, 0330-0400, 0400-0430, ... , 2600-2630, 2630-0300
	 * E.g.
	 * travelTimesFirstHalfTour[0] is the travel time for 0300hrs to 0330hrs (first half-hour window)
	 * travelTimesFirstHalfTour[47] is the travel time for 2630hrs to 0300hrs (last half-hour window)
	 *
	 */
	std::vector<int> travelTimesFirstHalfTour;
	std::vector<int> travelTimesSecondHalfTour;
	const unsigned int numTimeWindows;
};

} // end namespace medium
} // end namespace sim_mob
