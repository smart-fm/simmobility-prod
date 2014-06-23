//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "CalibrationStatistics.hpp"

#include <algorithm>
#include <functional>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include "util/CSVReader.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

namespace
{
const size_t SIZE_NUM_TOURS_STATS = 4; // num. of tours for a person is in [0,3]
const size_t SIZE_NUM_STOPS_STATS = 5; //num. of stops per tour can be {0, 1, 2, 3, 4+ }
const size_t SIZE_MODE_SHARE_STATS = 7; // there are 9 modes. Sum of all mode share percentages = 100%. Walk is omitted because we have a 1 degree of freedom. Shared rides are added up
const size_t SIZE_TRAVEL_DISTANCE_STATS = 8; // there are 8 classes of travel distances

const std::string NUM_0_TOURS = "num_0_tours";
const std::string NUM_1_TOURS = "num_1_tours";
const std::string NUM_2_TOURS = "num_2_tours";
const std::string NUM_3PLUS_TOURS = "num_3+_tours";
const std::string NUM_0_STOPS = "num_0_stops";
const std::string NUM_1_STOPS = "num_1_stops";
const std::string NUM_2_STOPS = "num_2_stops";
const std::string NUM_3_STOPS = "num_3_stops";
const std::string NUM_4PLUS_STOPS = "num_4+_stops";
const std::string NUM_TRIPS_0_5_KM = "num_trips_0_5_km";
const std::string NUM_TRIPS_5_10_KM = "num_trips_5_10_km";
const std::string NUM_TRIPS_10_15_KM = "num_trips_10_15_km";
const std::string NUM_TRIPS_15_20_KM = "num_trips_15_20_km";
const std::string NUM_TRIPS_20_25_KM = "num_trips_20_25_km";
const std::string NUM_TRIPS_25_30_KM = "num_trips_25_30_km";
const std::string NUM_TRIPS_30_40_KM = "num_trips_30_40_km";
const std::string NUM_TRIPS_40PLUS_KM = "num_trips_40+_km";
const std::string MODE_BUS_TOUR = "mode_bus_tour";
const std::string MODE_MRT_TOUR = "mode_mrt_tour";
const std::string MODE_PRIVATE_BUS_TOUR = "mode_private_bus_tour";
const std::string MODE_CAR_TOUR = "mode_car_tour";
const std::string MODE_CAR_PASSENGER_TOUR = "mode_car_passenger_tour";
const std::string MODE_MOTOR_BIKE_TOUR = "mode_motor_bike_tour";
const std::string MODE_TAXI_TOUR = "mode_taxi_tour";
const std::string MODE_BUS_STOP = "mode_bus_stop";
const std::string MODE_MRT_STOP = "mode_mrt_stop";
const std::string MODE_PRIVATE_BUS_STOP = "mode_private_bus_stop";
const std::string MODE_CAR_STOP = "mode_car_stop";
const std::string MODE_CAR_PASSENGER_STOP = "mode_car_passenger_stop";
const std::string MODE_MOTOR_BIKE_STOP = "mode_motor_bike_stop";
const std::string MODE_TAXI_STOP = "mode_taxi_stop";

size_t getModeIdx(int mode)
{
	size_t modeIdx;
	switch(mode)
	{
	case 1: // public bus
		modeIdx = 0;
		break;
	case 2: // MRT/LRT
		modeIdx = 1;
		break;
	case 3: // private bus
		modeIdx = 2;
		break;
	case 4: // drive 1
		modeIdx = 3;
		break;
	case 5: // shared 2
	case 6: // shared 3
		// Shared 2 and shared 3 are both car passenger
		modeIdx = 4;
		break;
	case 7: // motor bike
		modeIdx = 5;
		break;
	case 8: // walk
		// We are not collecting stats for walk
		return true;
	case 9: // taxi
		modeIdx = 6;
		break;
	}
	return modeIdx;
}
/**
 * computes mode share percentages
 * @param countVector input vector with individual counts
 * @param total total
 * @param pctVector output vector for percentages of tours
 * @return true if successful; false otherwise
 */
bool computePctVector(const std::vector<double>& countVector, const double total, std::vector<double>& pctVector)
{
	if(total <= 0 || countVector.empty()) { return false;}
	pctVector.clear();
	for(std::vector<double>::const_iterator i=countVector.begin(); i!=countVector.end(); i++)
	{
		pctVector.push_back((*i)/total);
	}
	return true;
}
}

sim_mob::medium::CalibrationStatistics::CalibrationStatistics()
: numPersonsWithTourCount(std::vector<double>(SIZE_NUM_TOURS_STATS,0)),
  numToursWithStopCount(std::vector<double>(SIZE_NUM_STOPS_STATS,0)),
  numTripsWithMode(std::vector<double>(SIZE_MODE_SHARE_STATS,0)),
  numToursWithMode(std::vector<double>(SIZE_MODE_SHARE_STATS,0)),
  numTripsWithDistance(std::vector<double>(SIZE_TRAVEL_DISTANCE_STATS,0)),
  totalTours(0), totalTrips(0), isSimulated(true)
{
}

sim_mob::medium::CalibrationStatistics::CalibrationStatistics(const std::string& observedValuesCSV_FileName)
: numPersonsWithTourCount(std::vector<double>(SIZE_NUM_TOURS_STATS, 0)),
  numToursWithStopCount(std::vector<double>(SIZE_NUM_STOPS_STATS, 0)),
  numTripsWithMode(std::vector<double>(SIZE_MODE_SHARE_STATS, 0)),
  numToursWithMode(std::vector<double>(SIZE_MODE_SHARE_STATS, 0)),
  numTripsWithDistance(std::vector<double>(SIZE_TRAVEL_DISTANCE_STATS, 0)),
  tourPctWithMode(std::vector<double>(SIZE_MODE_SHARE_STATS, 0)),
  tripPctWithMode(std::vector<double>(SIZE_MODE_SHARE_STATS, 0)),
  totalTours(0), totalTrips(0), isSimulated(false)
{
	CSV_Reader statsReader(observedValuesCSV_FileName, true);
	boost::unordered_map<std::string, std::string> observedValuesMap;
	statsReader.getNextRow(observedValuesMap, false);
	if (observedValuesMap.empty())
	{
		throw std::runtime_error("No data found for observed values of calibration statistics");
	}
	addToTourCountStats(0, boost::lexical_cast<double>(observedValuesMap.at(NUM_0_TOURS)));
	addToTourCountStats(1, boost::lexical_cast<double>(observedValuesMap.at(NUM_1_TOURS)));
	addToTourCountStats(2, boost::lexical_cast<double>(observedValuesMap.at(NUM_2_TOURS)));
	addToTourCountStats(3, boost::lexical_cast<double>(observedValuesMap.at(NUM_3PLUS_TOURS)));

	addToStopCountStats(0, boost::lexical_cast<double>(observedValuesMap.at(NUM_0_STOPS)));
	addToStopCountStats(1, boost::lexical_cast<double>(observedValuesMap.at(NUM_1_STOPS)));
	addToStopCountStats(2, boost::lexical_cast<double>(observedValuesMap.at(NUM_2_STOPS)));
	addToStopCountStats(3, boost::lexical_cast<double>(observedValuesMap.at(NUM_3_STOPS)));
	addToStopCountStats(4, boost::lexical_cast<double>(observedValuesMap.at(NUM_4PLUS_STOPS)));

	addToTravelDistanceStats(0, boost::lexical_cast<double>(observedValuesMap.at(NUM_TRIPS_0_5_KM)));
	addToTravelDistanceStats(1, boost::lexical_cast<double>(observedValuesMap.at(NUM_TRIPS_5_10_KM)));
	addToTravelDistanceStats(2, boost::lexical_cast<double>(observedValuesMap.at(NUM_TRIPS_10_15_KM)));
	addToTravelDistanceStats(3, boost::lexical_cast<double>(observedValuesMap.at(NUM_TRIPS_15_20_KM)));
	addToTravelDistanceStats(4, boost::lexical_cast<double>(observedValuesMap.at(NUM_TRIPS_20_25_KM)));
	addToTravelDistanceStats(5, boost::lexical_cast<double>(observedValuesMap.at(NUM_TRIPS_25_30_KM)));
	addToTravelDistanceStats(6, boost::lexical_cast<double>(observedValuesMap.at(NUM_TRIPS_30_40_KM)));
	addToTravelDistanceStats(7, boost::lexical_cast<double>(observedValuesMap.at(NUM_TRIPS_40PLUS_KM)));

	setTourModeSharePct(0, boost::lexical_cast<double>(observedValuesMap.at(MODE_BUS_TOUR)));
	setTourModeSharePct(1, boost::lexical_cast<double>(observedValuesMap.at(MODE_MRT_TOUR)));
	setTourModeSharePct(2, boost::lexical_cast<double>(observedValuesMap.at(MODE_PRIVATE_BUS_TOUR)));
	setTourModeSharePct(3, boost::lexical_cast<double>(observedValuesMap.at(MODE_CAR_TOUR)));
	setTourModeSharePct(4, boost::lexical_cast<double>(observedValuesMap.at(MODE_CAR_PASSENGER_TOUR)));
	setTourModeSharePct(5, boost::lexical_cast<double>(observedValuesMap.at(MODE_MOTOR_BIKE_TOUR)));
	setTourModeSharePct(6, boost::lexical_cast<double>(observedValuesMap.at(MODE_TAXI_TOUR)));

	setTripModeSharePct(0, boost::lexical_cast<double>(observedValuesMap.at(MODE_BUS_STOP)));
	setTripModeSharePct(1, boost::lexical_cast<double>(observedValuesMap.at(MODE_MRT_STOP)));
	setTripModeSharePct(2, boost::lexical_cast<double>(observedValuesMap.at(MODE_PRIVATE_BUS_STOP)));
	setTripModeSharePct(3, boost::lexical_cast<double>(observedValuesMap.at(MODE_CAR_STOP)));
	setTripModeSharePct(4, boost::lexical_cast<double>(observedValuesMap.at(MODE_CAR_PASSENGER_STOP)));
	setTripModeSharePct(5, boost::lexical_cast<double>(observedValuesMap.at(MODE_MOTOR_BIKE_STOP)));
	setTripModeSharePct(6, boost::lexical_cast<double>(observedValuesMap.at(MODE_TAXI_STOP)));

}

sim_mob::medium::CalibrationStatistics::~CalibrationStatistics()
{
}

void sim_mob::medium::CalibrationStatistics::reset()
{
	numPersonsWithTourCount = std::vector<double>(SIZE_NUM_TOURS_STATS,0);
	numToursWithStopCount = std::vector<double>(SIZE_NUM_STOPS_STATS,0);
	numToursWithMode = std::vector<double>(SIZE_MODE_SHARE_STATS, 0);
	numTripsWithMode = std::vector<double>(SIZE_MODE_SHARE_STATS,0);
	numTripsWithDistance = std::vector<double>(SIZE_TRAVEL_DISTANCE_STATS,0);
	totalTrips = 0;
	totalTours = 0;
}

void sim_mob::medium::CalibrationStatistics::getAllStatistics(std::vector<double>& outStatistics)
{

	outStatistics.insert(outStatistics.end(), numPersonsWithTourCount.begin(), numPersonsWithTourCount.end());
	outStatistics.insert(outStatistics.end(), numToursWithStopCount.begin(), numToursWithStopCount.end());
	if(isSimulated)
	{
		computePctVector(numToursWithMode, totalTours, tourPctWithMode);
		outStatistics.insert(outStatistics.end(), tourPctWithMode.begin(), tourPctWithMode.end());
		computePctVector(numTripsWithMode, totalTrips, tripPctWithMode);
		outStatistics.insert(outStatistics.end(), tripPctWithMode.begin(), tripPctWithMode.end());
	}
	else
	{
		outStatistics.insert(outStatistics.end(), tourPctWithMode.begin(), tourPctWithMode.end());
		outStatistics.insert(outStatistics.end(), tripPctWithMode.begin(), tripPctWithMode.end());
	}
	outStatistics.insert(outStatistics.end(), numTripsWithDistance.begin(), numTripsWithDistance.end());
}

bool sim_mob::medium::CalibrationStatistics::addToTourCountStats(size_t numTours, double toAdd)
{
	if(numTours < numPersonsWithTourCount.size())
	{
		numPersonsWithTourCount[numTours] = numPersonsWithTourCount[numTours] + toAdd;
		return true;
	}
	return false;
}

bool sim_mob::medium::CalibrationStatistics::addToStopCountStats(size_t numStops, double toAdd)
{
	if(numStops > 4) { numStops = 4; } // 4+
	numToursWithStopCount[numStops] = numToursWithStopCount[numStops] + toAdd;
	return true;
}

bool sim_mob::medium::CalibrationStatistics::addToTripModeShareStats(int mode, double toAdd)
{
	if(mode > 9 || mode < 1) { return false; }
	size_t modeIdx = getModeIdx(mode);
	numTripsWithMode[modeIdx] = numTripsWithMode[modeIdx] + toAdd;
	totalTrips = totalTrips + toAdd;
	return true;
}

bool sim_mob::medium::CalibrationStatistics::addToTourModeShareStats(int mode, double toAdd)
{
	if(mode > 9 || mode < 1) { return false; }
	size_t modeIdx = getModeIdx(mode);
	numToursWithMode[modeIdx] = numToursWithMode[modeIdx] + toAdd;
	totalTours = totalTours + toAdd;
	return true;
}


bool sim_mob::medium::CalibrationStatistics::setTripModeSharePct(size_t modeIdx, double toAdd)
{
	tripPctWithMode[modeIdx] = toAdd;
	return true;
}

bool sim_mob::medium::CalibrationStatistics::setTourModeSharePct(size_t modeIdx, double toAdd)
{
	tourPctWithMode[modeIdx] = toAdd;
	return true;
}

bool sim_mob::medium::CalibrationStatistics::addToTravelDistanceStats(double distance, double toAdd)
{
	if(distance < 0) { return false; } // inadmissible
	else if(distance >= 40) { numTripsWithDistance[7] = numTripsWithDistance[7] + toAdd; } // [40,infinity) km
	else if(distance >= 30) { numTripsWithDistance[6] = numTripsWithDistance[6] + toAdd; } // [30, 40) km
	else if(distance >= 25) { numTripsWithDistance[5] = numTripsWithDistance[5] + toAdd; } // [25, 30) km
	else if(distance >= 20) { numTripsWithDistance[4] = numTripsWithDistance[4] + toAdd; } // [20, 25) km
	else if(distance >= 15) { numTripsWithDistance[3] = numTripsWithDistance[3] + toAdd; } // [15, 20) km
	else if(distance >= 10) { numTripsWithDistance[2] = numTripsWithDistance[2] + toAdd; } // [10, 15) km
	else if(distance >= 5) { numTripsWithDistance[1] = numTripsWithDistance[1] + toAdd; }  // [5 , 10) km
	else { numTripsWithDistance[0] = numTripsWithDistance[0] + toAdd; }                    // [0 , 5 ) km
	return true;
}

CalibrationStatistics& sim_mob::medium::CalibrationStatistics::operator+(const CalibrationStatistics& rightOperand)
{
	std::transform(numPersonsWithTourCount.begin(), numPersonsWithTourCount.end(),
			rightOperand.numPersonsWithTourCount.begin(), numPersonsWithTourCount.begin(), std::plus<double>());

	std::transform(numToursWithStopCount.begin(), numToursWithStopCount.end(),
			rightOperand.numToursWithStopCount.begin(), numToursWithStopCount.begin(), std::plus<double>());

	std::transform(numToursWithMode.begin(), numToursWithMode.end(),
			rightOperand.numToursWithMode.begin(), numToursWithMode.begin(), std::plus<double>());

	std::transform(numTripsWithMode.begin(), numTripsWithMode.end(),
			rightOperand.numTripsWithMode.begin(), numTripsWithMode.begin(), std::plus<double>());

	std::transform(numTripsWithDistance.begin(), numTripsWithDistance.end(),
			rightOperand.numTripsWithDistance.begin(), numTripsWithDistance.begin(), std::plus<double>());

	return *this;  // Return a reference to myself.
}
