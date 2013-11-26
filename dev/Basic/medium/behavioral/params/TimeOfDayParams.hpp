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
 * Simple class to store information pertaining tour time of day models
 * \note This class is used by the mid-term behavior models.
 *
 * \author Harish Loganathan
 */
class TourTimeOfDayParams {
public:
	TourTimeOfDayParams() : numTimeWindows(48) {}
	virtual ~TourTimeOfDayParams() {}

	/**
	 * Templated function to get a specific element from first half tour vector travelTimesFirstHalfTour
	 *
	 * @returns the value from the vector if index < 48; returns -1 otherwise;
	 * \note -1 is an invalid value for travel time and the caller must check for this value.
	 */
	template <unsigned index>
	double getTT_FirstHalfTour(){
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
	double getTT_SecondHalfTour(){
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
	 */
	std::vector<double> travelTimesFirstHalfTour;
	std::vector<double> travelTimesSecondHalfTour;
	const unsigned int numTimeWindows;

};

/**
 * class to store parameters for the stop time of day model
 * \note This class is used by the mid-term behavior models.
 *
 * \author Harish Loganathan
 */
class StopTimeOfDayParams {
public:
	StopTimeOfDayParams(int stopType, int firstBound) : stopType(stopType), firstBound(firstBound), numTimeWindows(48), todHigh(0.0), todLow(0.0) {
		for(double i=3.25; i<27; i=i+0.5) {
			availability[i] = true;
		}
	}
	virtual ~StopTimeOfDayParams() {}

	int getFirstBound() const {
		return firstBound;
	}

	int getSecondBound() const {
		return !firstBound;
	}

	int getStopType() const {
		return stopType;
	}

	double getTodHigh() const {
		return todHigh;
	}

	void setTodHigh(double todHigh) {
		this->todHigh = todHigh;
	}

	double getTodLow() const {
		return todLow;
	}

	void setTodLow(double todLow) {
		this->todLow = todLow;
	}

	/**
	 * Templated function to get a specific element from vector travelTimes
	 *
	 * @returns the value from the vector if index < 48; returns -1 otherwise;
	 * \note -1 is an invalid value for travel time and the caller must check for this value.
	 */
	template <unsigned index>
	double getTravelTime(){
		if(index < numTimeWindows) {
			return travelTimes[index];
		}
		return -1;
	}

	/**
	 * Templated function to get a specific element from vector travelCost
	 *
	 * @returns the value from the vector if index < 48; returns -1 otherwise;
	 * \note -1 is an invalid value for travel time and the caller must check for this value.
	 */
	template <unsigned index>
	double getTravelCost(){
		if(index < numTimeWindows) {
			return travelCost[index];
		}
		return -1;
	}

	/**
	 * Templated function to get the availability of an alternative
	 */
	template <unsigned index>
	int getAvailability(){
		if(index < numTimeWindows) {
			return availability[index];
		}
		return -1;
	}

	/**
	 * Vectors storing travel times and travel costs for each half-hour time window from 3.25 to 26.75
	 *
	 * E.g.
	 * travelTimes[3] is the travel time for 4.25 window
	 * travelCost[47] is the travel cost for 26.75 window
	 */
	std::vector<double> travelTimes;
	std::vector<double> travelCost;
	std::vector<bool> availability;

private:
	int stopType;
	int firstBound;
	double todHigh; // upper limit for time of day for this stop
	double todLow; // lower limit for time of day for this stop
	int numTimeWindows;
};
} // end namespace medium
} // end namespace sim_mob
