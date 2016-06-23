//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <boost/unordered_map.hpp>
#include <boost/function.hpp>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include "behavioral/params/PersonParams.hpp"
#include "behavioral/params/ZoneCostParams.hpp"
#include "CalibrationStatistics.hpp"
#include "config/MT_Config.hpp"
#include "PredaySystem.hpp"
#include "PredayClasses.hpp"

namespace sim_mob
{
namespace medium
{

/**
 * structure to hold a calibration variable and its pertinent details.
 *
 * \author Harish Loganathan
 */
struct CalibrationVariable
{
public:
	CalibrationVariable() :
			variableName(std::string()), scriptFileName(std::string()), initialValue(0), currentValue(0), lowerLimit(0), upperLimit(0)
	{
	}

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

/**
 * Manager class which handles all preday level computations
 *
 * \author Harish Loganathan
 */
class PredayManager
{
public:

	PredayManager();
	virtual ~PredayManager();

	/**
	 * Gets person ids of each person in the population data
	 *
	 * @param dbType type of backend where the population data is available
	 */
	void loadPersonIds(db::BackendType dbType);

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
	 * Gets mapping of postcode to nearest node
	 *
	 * @param dbType type of backend where the zone node mapping data is available
	 */
	void loadPostcodeNodeMapping(db::BackendType dbType);

	/**
	 * loads the AM, PM and off peak costs data
	 *
	 * @param dbType type of backend where the cost data is available
	 */
	void loadCosts(db::BackendType dbType);

	/**
	 * loads the un-available origin destination pairs
	 *
	 * @param dbType type of backend where the cost data is available
	 */
	void loadUnavailableODs(db::BackendType dbType);

	/**
	 * Distributes mongodb persons to different threads and starts the threads which process the persons
	 */
	void dispatchMongodbPersons();

	/**
	 * Distributes long-term persons to different threads and starts the threads which process the persons
	 */
	void dispatchLT_Persons();

	/**
	 * preday calibration function
	 */
	void calibratePreday();

private:
	typedef std::vector<PersonParams*> PersonList;
	typedef boost::unordered_map<int, ZoneParams*> ZoneMap;
	typedef boost::unordered_map<int, boost::unordered_map<int, CostParams*> > CostMap;
	typedef boost::unordered_map<int, std::vector<ZoneNodeParams*> > ZoneNodeMap;
	typedef std::vector<std::string> PersonIdList;
	typedef std::vector<long> LT_PersonIdList;

	typedef void (PredayManager::*threadedFnPtr)(const PersonList::iterator&, const PersonList::iterator&, size_t);

	/**
	 * Threaded function loop for simulation.
	 * Loops through all elements in personList within the specified range and
	 * invokes the Preday system of models for each of them.
	 *
	 * @param first personIdList iterator corresponding to the first person to be
	 * 				processed
	 * @param last personIdList iterator corresponding to the person after the
	 * 				last person to be processed
	 */
	void processPersons(const PersonIdList::iterator& first, const PersonIdList::iterator& last, const std::string& scheduleLog);

	/**
	 * Threaded function loop for simulation of LT population
	 * Loops through all elements in personList within the specified range and
	 * invokes the Preday system of models for each of them.
	 *
	 * @param first personIdList iterator corresponding to the first person to be
	 * 				processed
	 * @param last personIdList iterator corresponding to the person after the
	 * 				last person to be processed
	 */
	void processPersonsForLT_Population(const LT_PersonIdList::iterator& first, const LT_PersonIdList::iterator& last, const std::string& scheduleLog);

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
	 * Loops through all elements in personIdList within the specified range and
	 * invokes logsum computations for each of them.
	 *
	 * @param first personIdList iterator corresponding to the first person to be
	 * 				processed
	 * @param last personIdList iterator corresponding to the person after the
	 * 				last person to be processed
	 */
	void computeLogsums(const PersonIdList::iterator& firstPersonIdIt, const PersonIdList::iterator& oneAfterLastPersonIdIt);

	/**
	 * Threaded logsum computation for LT population
	 * Loops through all elements in personIdList within the specified range and
	 * invokes logsum computations for each of them.
	 *
	 * @param first personIdList iterator corresponding to the first person to be
	 * 				processed
	 * @param last personIdList iterator corresponding to the person after the
	 * 				last person to be processed
	 */
	void computeLogsumsForLT_Population(const LT_PersonIdList::iterator& firstPersonIdIt, const LT_PersonIdList::iterator& oneAfterLastPersonIdIt);

	/**
	 * Threaded logsum computation for LT feedback.
	 * Loops through all elements in personIdList within the specified range and
	 * invokes logsum computations for each of them.
	 *
	 * @param first personIdList iterator corresponding to the first person to be
	 * 				processed
	 * @param last personIdList iterator corresponding to the person after the
	 * 				last person to be processed
	 *
	 * \NOTE: This function must be removed when we are able to fully feedback LT population's logsums
	 */
	void computeLT_FeedbackLogsums(const PersonIdList::iterator& firstPersonIdIt, const PersonIdList::iterator& oneAfterLastPersonIdIt,
			const std::string& logsumOutputFileName);

	/**
	 * Threaded logsum computation for LT feedback.
	 * Loops through all elements in personIdList within the specified range and
	 * invokes logsum computations for each of them.
	 *
	 * @param first personIdList iterator corresponding to the first person to be
	 * 				processed
	 * @param last personIdList iterator corresponding to the person after the
	 * 				last person to be processed
	 *
	 * \NOTE: This function must be removed when we are able to fully feedback LT population's logsums
	 */
	void computeLT_PopulationFeedbackLogsums(const LT_PersonIdList::iterator& firstPersonIdIt, const LT_PersonIdList::iterator& oneAfterLastPersonIdIt,
			const std::string& logsumOutputFileName);

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
	 *
	 * @return computed objective function result
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

	/**
	 * list of persons to process
	 */
	PersonList personList;

	/**
	 * list of person Ids
	 */
	PersonIdList personIdList;

	/**
	 * list of LT person Ids
	 */
	LT_PersonIdList ltPersonIdList;

	/**
	 * map of zone_id -> ZoneParams
	 */
	ZoneMap zoneMap;
	ZoneNodeMap zoneNodeMap;
	boost::unordered_map<int, int> zoneIdLookup;
	std::map<int, int> MTZ12_MTZ08_Map;

	/**
	 * Map of AM Costs [origin zone, destination zone] -> CostParams*
	 */
	CostMap amCostMap;

	/**
	 * Map of PM Costs [origin zone, destination zone] -> CostParams*
	 */
	CostMap pmCostMap;

	/**
	 * Map of Off peak Costs [origin zone, destination zone] -> CostParams*
	 */
	CostMap opCostMap;

	/** for each origin, has a list of unavailable destinations */
	std::vector<OD_Pair> unavailableODs;

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

	/**
	 * mid-term configurations
	 */
	const MT_Config& mtConfig;

	/**
	 * output file for calibration results
	 */
	std::ostream* logFile;

	/**
	 * stream for logging calibration results
	 */
	std::stringstream logStream;

};
} //end namespace medium
} //end namespace sim_mob

