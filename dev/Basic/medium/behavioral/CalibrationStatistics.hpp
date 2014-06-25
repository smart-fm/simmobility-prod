//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <boost/unordered_map.hpp>
#include <cstdlib>
#include <string>
#include <vector>

namespace sim_mob
{
namespace medium
{
/**
 * class to track the statistics needed for calibration
 *
 * \author Harish Loganathan
 */
class CalibrationStatistics
{
private:
	//city level number of tours and trips
	/**
	 * vector to track the number of persons with different number of tours.
	 * Spoken semantics apply.
	 * e.g. numPersonsWithTourCount[3] is the number of persons who have 3 tours.
	 */
	std::vector<double> numPersonsWithTourCount;

	/**
	 * vector to track the number of tours with different number of stops
	 * Spoken semantics apply.
	 * e.g. numToursWithStopCount[3] is the number of tours with 3 stops
	 *      numToursWithStopCount[4] is the number of tours with 4+ stops
	 */
	std::vector<double> numToursWithStopCount;

	/**
	 * mode share count for trips
	 * Lookup:
	 * -- 0 for public bus; 1 for MRT/LRT; 2 for private bus; 3 for drive1;
	 * -- 4 for car passenger; 5 for motor; 6 for taxi
	 *
	 * e.g. numTripsWithMode[5] is the number of trips with motor
	 */
	std::vector<double> numTripsWithMode;

	/**
	 * mode share statistics for tours
	 * Lookup:
	 * -- 0 for public bus; 1 for MRT/LRT; 2 for private bus; 3 for drive1;
	 * -- 4 for car passenger; 5 for motor; 6 for taxi
	 *
	 * e.g. numToursWithMode[5] is the number of tours with motor bike as its primary mode
	 */
	std::vector<double> numToursWithMode;

	/**
	 * mode share percentage for trips
	 * Lookup:
	 * -- 0 for public bus; 1 for MRT/LRT; 2 for private bus; 3 for drive1;
	 * -- 4 for car passenger; 5 for motor; 6 for taxi
	 *
	 * e.g. tripPctWithMode[5] is the number of trips with motor
	 */
	std::vector<double> tripPctWithMode;

	/**
	 * mode share percentage for tours
	 * Lookup:
	 * -- 0 for public bus; 1 for MRT/LRT; 2 for private bus; 3 for drive1;
	 * -- 4 for car passenger; 5 for motor; 6 for taxi
	 *
	 * e.g. tourPctWithMode[5] is the number of tours with motor bike as its primary mode
	 */
	std::vector<double> tourPctWithMode;

	/**
	 * trip distance statistics
	 * Lookup:
	 * -- 0 [0,5) km; 1 [5,10) km; 2 [10,15) km; 3 [15,20) km; 4 [20,25) km; 5 [25,30) km;
	 * -- 6 [30,40) km; 7 [40,infinity) km;
	 *
	 * e.g. numTripsWithDistance[1] is the number of trips with travel distance in range [5,10)
	 */
	std::vector<double> numTripsWithDistance;

	/**total number of trips*/
	double totalTrips;

	/**total number of tours*/
	double totalTours;

	/**flag to indicate if the collected statistics are observed or simulated */
	bool isSimulated;

	/**
	 * sets percentage of trips with 'mode' as mode.
	 * Lookup:
	 * -- 0 for public bus; 1 for MRT/LRT; 2 for private bus; 3 for drive1;
	 * -- 4 for car passenger; 5 for motor; 6 for taxi
	 *
	 * @param mode the mode in [0,6] to set
	 * @param toAdd the number to add to corresponding statistic
	 * @return true if addition was successful; false otherwise
	 */
	bool setTripModeSharePct(size_t mode, double toAdd);

	/**
	 * sets percentage of tours with 'mode' as primary mode.
	 * Expected modes: [1,9] as used by the preday models
	 * Lookup:
	 * -- 0 for public bus; 1 for MRT/LRT; 2 for private bus; 3 for drive1;
	 * -- 4 for car passenger; 5 for motor; 6 for taxi
	 *
	 * @param mode the mode in [0,6] to set
	 * @param toAdd the number to add to corresponding statistic
	 * @return true if addition was successful; false otherwise
	 */
	bool setTourModeSharePct(size_t mode, double toAdd);

public:
	CalibrationStatistics();
	CalibrationStatistics(const boost::unordered_map<std::string, std::string>& observedValuesCSV_FileName);
	virtual ~CalibrationStatistics();

	/**
	 * resets all vectors in this class
	 */
	void reset();

	/**
	 * concatenates all statistics into outStatistics
	 * @param outStatistics vector to be populated with all statistics
	 */
	void getAllStatistics(std::vector<double>& outStatistics);

	/**
	 * adds to the current number of persons with 'numTours' tours
	 * @param numTours number of tours to add to
	 * @param toAdd the number to add to corresponding statistic
	 * @return true if addition was successful; false otherwise
	 */
	bool addToTourCountStats(size_t numTours, double toAdd);

	/**
	 * adds to the current number of tours with 'numStops' stops
	 * @param numStops number of stops to add to
	 * @param toAdd the number to add to corresponding statistic
	 * @return true if addition was successful; false otherwise
	 */
	bool addToStopCountStats(size_t numStops, double toAdd);

	/**
	 * adds to the current number of trips with 'mode' as mode.
	 * Expected modes: [1,9] as used by the preday models
	 * -- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
	 * -- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
	 *
	 * @param mode the mode in [1,9] to add to
	 * @param toAdd the number to add to corresponding statistic
	 * @return true if addition was successful; false otherwise
	 */
	bool addToTripModeShareStats(int mode, double toAdd);

	/**
	 * adds to the current number of tours with 'mode' as primary mode.
	 * Expected modes: [1,9] as used by the preday models
	 * -- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
	 * -- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
	 *
	 * @param mode the mode in [1,9] to add to
	 * @param toAdd the number to add to corresponding statistic
	 * @return true if addition was successful; false otherwise
	 */
	bool addToTourModeShareStats(int mode, double toAdd);

	/**
	 * adds to the current number of trips with 'mode' as mode.
	 * @param mode the mode to add to
	 * @param toAdd the number to add to corresponding statistic
	 * @return true if addition was successful; false otherwise
	 */
	bool addToTravelDistanceStats(double distance, double toAdd);

	/**
	 * Adds corresponding values in all vectors of rightOperand to this
	 * if both operands have simulated statistics
	 * @param rightOperand the right operand of addition (*this* is left)
	 * @return
	 */
	CalibrationStatistics& operator+(const CalibrationStatistics& rightOperand);

	/** pretty prints this object */
	void prettyPrint();
};
}
}

