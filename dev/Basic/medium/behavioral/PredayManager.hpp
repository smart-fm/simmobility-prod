//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayManager.hpp
 *
 *  Created on: Nov 18, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include <boost/unordered_map.hpp>
#include <boost/function.hpp>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include "behavioral/PredaySystem.hpp"
#include "CalibrationStatistics.hpp"
#include "config/MT_Config.hpp"
#include "params/PersonParams.hpp"
#include "params/ZoneCostParams.hpp"
#include "database/DB_Connection.hpp"

namespace sim_mob {
namespace medium {

/**
 * structure to hold a calibration variable and its pertinent details.
 */
struct CalibrationVariable
{
public:
	CalibrationVariable() : variableName(std::string()), scriptFileName(std::string()), initialValue(0), currentValue(0), lowerLimit(0), upperLimit(0)
	{}

	double getInitialValue() const
	{
		return initialValue;
	}

	double getLowerLimit() const
	{
		return lowerLimit;
	}

	const std::string& getScriptFileName() const
	{
		return scriptFileName;
	}

	double getUpperLimit() const
	{
		return upperLimit;
	}

	const std::string& getVariableName() const
	{
		return variableName;
	}

	void setInitialValue(double initialValue)
	{
		this->initialValue = initialValue;
	}

	void setLowerLimit(double lowerLimit)
	{
		this->lowerLimit = lowerLimit;
	}

	void setScriptFileName(const std::string& scriptFileName)
	{
		this->scriptFileName = scriptFileName;
	}

	void setUpperLimit(double upperLimit)
	{
		this->upperLimit = upperLimit;
	}

	void setVariableName(const std::string& variableName)
	{
		this->variableName = variableName;
	}

	double getCurrentValue() const
	{
		return currentValue;
	}

	void setCurrentValue(double currentValue)
	{
		this->currentValue = currentValue;
	}

private:
	std::string variableName;
	std::string scriptFileName;
	double initialValue;
	double currentValue;
	double lowerLimit;
	double upperLimit;
};

class PredayManager {
public:

	PredayManager();
	virtual ~PredayManager();

	/**
	 * Gets person data from the database and stores corresponding PersonParam pointers in personList.
	 *
	 * @param dbType type of backend where the population data is available
	 */
	void loadPersons(db::BackendType dbType);

	/**
	 * Gets details of all mtz zones
	 *
	 * @param dbType type of backend where the zone data is available
	 */
	void loadZones(db::BackendType dbType);

	/**
	 * Gets the list of nodes within each zone and stores them in a map
	 *
	 * @param dbType type of backend where the zone node mapping data is available
	 */
	void loadZoneNodes(db::BackendType dbType);

	/**
	 * loads the AM, PM and off peak costs data
	 *
	 * @param dbType type of backend where the cost data is available
	 */
	void loadCosts(db::BackendType dbType);

	/**
	 * Distributes persons to different threads and starts the threads which process the persons
	 */
	void dispatchPersons();

	/**
	 * preday calibration function
	 */
	void calibratePreday();

private:
	typedef std::vector<PersonParams*> PersonList;
	typedef boost::unordered_map<int, ZoneParams*> ZoneMap;
	typedef boost::unordered_map<int, boost::unordered_map<int, CostParams*> > CostMap;
	typedef boost::unordered_map<int, std::vector<ZoneNodeParams*> > ZoneNodeMap;

	typedef void (PredayManager::*threadedFnPtr)(const PersonList::iterator&, const PersonList::iterator&, size_t);

	/**
	 * Threaded function loop for simulation.
	 * Loops through all elements in personList within the specified range and
	 * invokes the Preday system of models for each of them.
	 *
	 * @param first personList iterator corresponding to the first person to be
	 * 				processed
	 * @param last personList iterator corresponding to the person after the
	 * 				last person to be processed
	 */
	void processPersons(const PersonList::iterator& first, const PersonList::iterator& last, const std::string& tripChainLog);

	/**
	 * Distributes persons to different threads and starts the threads which process the persons for calibration
	 */
	void distributeAndProcessForCalibration(threadedFnPtr fnPtr);

	/**
	 * Threaded function loop for calibration.
	 * Loops through all elements in personList within the specified range and
	 * invokes the Preday system of models for each of them.
	 *
	 * @param first personList iterator corresponding to the first person to be
	 * 				processed
	 * @param last personList iterator corresponding to the person after the
	 * 				last person to be processed
	 * @param simStats the object to collect statistics into
	 */
	void processPersonsForCalibration(const PersonList::iterator& first, const PersonList::iterator& last, size_t threadNum);

	/**
	 * Threaded logsum computation
	 * Loops through all elements in personList within the specified range and
	 * invokes logsum computations for each of them.
	 *
	 * @param first personList iterator corresponding to the first person to be
	 * 				processed
	 * @param last personList iterator corresponding to the person after the
	 * 				last person to be processed
	 */
	void computeLogsums(const PersonList::iterator& first, const PersonList::iterator& last);

	/**
	 * Threaded logsum computation for calibration
	 * Loops through all elements in personList within the specified range and
	 * invokes logsum computations for each of them.
	 * This function does not update new logsums in mongodb. Updates only in memory.
	 *
	 * @param first personList iterator corresponding to the first person to be
	 * 				processed
	 * @param last personList iterator corresponding to the person after the
	 * 				last person to be processed
	 */
	void computeLogsumsForCalibration(const PersonList::iterator& firstPersonIt, const PersonList::iterator& oneAfterLastPersonIt, size_t threadNum);

	/**
	 * updates logsums in mongodb
	 */
	void updateLogsumsToMongoAfterCalibration(const PersonList::iterator& firstPersonIt, const PersonList::iterator& oneAfterLastPersonIt, size_t threadNum);

	/**
	 * loads csv containing calibration variables for preday
	 */
	void loadCalibrationVariables();

	/**
	 * loads csv containing the weight matrix
	 */
	void loadWeightMatrix();

	/**
	 * computes gradient vector
	 *
	 * @param randomVector symmetric random vector of +1s and -1s
	 * @param initialGradientStepSize initial gradient step size
	 * @param gradientVector output vector to be populated
	 */
	void computeGradient(const std::vector<short>& randomVector, double initialGradientStepSize, std::vector<double>& gradientVector);

	/**
	 * computes gradient vector accomodating weights
	 *
	 * @param randomVector symmetric random vector of +1s and -1s
	 * @param initialGradientStepSize initial gradient step size
	 * @param gradientVector output vector to be populated
	 */
	void computeWeightedGradient(const std::vector<short>& randomVector, double initialGradientStepSize, std::vector<double>& gradientVector);

	/**
	 * runs preday and computes the value for objective function
	 * @param calibrationVariableList values of calibration variables to use in simulation
	 * @param simulatedHITS_Stats output list to populate simulated stats
	 */
	double computeObjectiveFunction(const std::vector<CalibrationVariable>& calibrationVariableList, std::vector<double>& simulatedHITS_Stats);

	/**
	 * runs preday and computes the value for objective function
	 * @param calibrationVariableList values of calibration variables to use in simulation
	 * @param simulatedHITS_Stats output list to populate
	 */
	void computeObservationsVector(const std::vector<CalibrationVariable>& calVarList, std::vector<double>& simulatedHITS_Stats);

	/**
	 * logs logStream into file
	 */
	void log();

	PersonList personList;

    /**
     * map of zone_id -> ZoneParams
     * \note this map has 1092 elements
     */
    ZoneMap zoneMap;
    ZoneNodeMap zoneNodeMap;
    boost::unordered_map<int,int> zoneIdLookup;

    /**
     * Map of AM, PM and Off peak Costs [origin zone, destination zone] -> CostParams*
     * \note these maps have (1092 zones * 1092 zones - 1092 (entries with same origin and destination is not available)) 1191372 elements
     */
    CostMap amCostMap;
    CostMap pmCostMap;
    CostMap opCostMap;

    /**
     * list of values computed for objective function
     * objectiveFunctionValue[i] is the objective function value for iteration i
     */
    std::vector<double> objectiveFunctionValues;

    /**
     * Calibration statistics collector for each thread
     * simulatedStats[4] is the statistics from the 5th thread
     */
    std::vector<CalibrationStatistics> simulatedStatsVector;

    const MT_Config& mtConfig;

    std::ostream* logFile;

    std::stringstream logStream;

};
} //end namespace medium
} //end namespace sim_mob



