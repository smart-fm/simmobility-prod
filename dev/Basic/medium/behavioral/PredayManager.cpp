//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayManager.cpp
 *
 *  Created on: Nov 18, 2013
 *      Author: Harish Loganathan
 */

#include "PredayManager.hpp"

#include <algorithm>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Config.hpp"
#include "database/PopulationSqlDao.hpp"
#include "database/PopulationMongoDao.hpp"
#include "database/TripChainSqlDao.hpp"
#include "database/ZoneCostMongoDao.hpp"
#include "util/CSVReader.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob;
using namespace sim_mob::db;
using namespace sim_mob::medium;
using namespace boost::numeric::ublas;
namespace
{
//Header strings used in CSV file for calibration variables
const std::string VARIABLE_NAME_CSV_HEADER = "variable_name";
const std::string LUA_FILE_NAME_CSV_HEADER = "lua_file_name";
const std::string INITIAL_VALUE_CSV_HEADER = "initial_value";
const std::string LOWER_LIMIT_CSV_HEADER = "lower_limit";
const std::string UPPER_LIMIT_CSV_HEADER = "upper_limit";

/** vector of variables to be calibrated. used only in calibration mode of Preday*/
std::vector<CalibrationVariable> calibrationVariablesList;
size_t numVariablesCalibrated;

/**Mongo daos for calibration*/
boost::unordered_map<std::string, db::MongoDao*> mongoDao;

//initialize once and use multiple times
std::vector<double> observedHITS_Stats;

matrix<double> weightMatrix;

/**
 * Helper class for symmetric random vector of +1/-1
 * An element of the vector can be either +1 or -1 with probability 1/2 (symmetric)
 * The elements of the vector are independent and identically distributed (IID)
 * The size of the vector is the number of calibrated variables
 *
 */
struct RandomSymmetricPlusMinusVector
{
public:
	RandomSymmetricPlusMinusVector(size_t dimension) : dimension(dimension)
	{
		//set the seed
		srand(time(0));
	}

	/**
	 * populates randomVector by Bernouli's process
	 * each element is +1 or -1 with probability 0.5
	 */
	void init()
	{
		randomVector.clear();
		for(size_t i=0; i < dimension; i++)
		{
			if((rand() % 2) == 0){ randomVector.push_back(1); }
			else { randomVector.push_back(-1); }
		}
	}

	/**
	 * gets a new random vector on every call
	 * the vector is a random composition of either +1 or -1
	 */
	const std::vector<short>& get()
	{
		init();
		return randomVector;
	}

private:
	/**the of the random vector*/
	size_t dimension;
	/**symmetrical random vector of 1s and -1s*/
	std::vector<short> randomVector;
};

/**
 * helper function to compute Anti gradient
 * @param gradientVector the gradient vector
 * @param outAntiGradientVector the anti-gradient vector to populate
 */
void computeAntiGradientVector(const std::vector<double>& gradientVector, std::vector<double>& outAntiGradientVector)
{
	if(gradientVector.size() != calibrationVariablesList.size())
	{
		throw std::runtime_error("gradientVector.size() != calibrationVariablesList.size()");
	}
	outAntiGradientVector.clear();
	double gradient = 0;
	double antiGradient = 0;
	size_t variableIdx = 0;
	for(std::vector<double>::const_iterator gradIt=gradientVector.begin(); gradIt!=gradientVector.end(); gradIt++, variableIdx++)
	{
		gradient = *gradIt;
		const CalibrationVariable& calVar = calibrationVariablesList[variableIdx];
		if((gradient > 0 && calVar.getLowerLimit() == calVar.getCurrentValue()) ||
				(gradient < 0 && calVar.getUpperLimit() == calVar.getCurrentValue()))
		{
			antiGradient = 0;
		}
		else
		{
			antiGradient = -gradient;
		}
		outAntiGradientVector.push_back(antiGradient);
	}
}

/**
 * helper function to multiply a scalar and a vector
 */
template<typename V>
void multiplyScalarWithVector(double scalarLhs, const std::vector<V>& vectorRhs, std::vector<double>& output)
{
	for(typename std::vector<V>::const_iterator vIt=vectorRhs.begin(); vIt!=vectorRhs.end(); vIt++)
	{
		double result = (*vIt) * scalarLhs;
		output.push_back(result);
	}
}

void updateVariablesByAddition(std::vector<CalibrationVariable>& calVarList, const std::vector<double>& rhs)
{
	if(calVarList.size() != rhs.size()) { throw std::runtime_error("cannot add vectors of different sizes"); }
	size_t vectorSize = calVarList.size();
	for(size_t i=0; i<vectorSize; i++)
	{
		CalibrationVariable& calVar = calVarList[i];
		calVar.setCurrentValue(calVar.getCurrentValue()+rhs[i]);
	}
}

void updateVariablesBySubtraction(std::vector<CalibrationVariable>& calVarList, const std::vector<double>& rhs)
{
	if(calVarList.size() != rhs.size()) { throw std::runtime_error("cannot subtract vectors of different sizes"); }
	size_t vectorSize = calVarList.size();
	for(size_t i=0; i<vectorSize; i++)
	{
		CalibrationVariable& calVar = calVarList[i];
		calVar.setCurrentValue(calVar.getCurrentValue()-rhs[i]);
	}
}

void updateVariablesInLuaFiles(const std::string& scriptFilesPath, const std::vector<CalibrationVariable>& calVarList)
{
	std::stringstream cmdStream;
	int result = 0;
	for (std::vector<CalibrationVariable>::const_iterator calVarIt = calVarList.begin(); calVarIt != calVarList.end(); calVarIt++)
	{
		const CalibrationVariable& calVar = *calVarIt;
		const std::string origFileName(scriptFilesPath + calVar.getScriptFileName());
		const std::string tmpFileName(origFileName + ".tmp");
		cmdStream.str(std::string());
		cmdStream << "sed -e 's/local " << calVar.getVariableName() << ".*=.*[0-9]/local " << calVar.getVariableName() << "= " << calVar.getCurrentValue()
				<< "/g' " << origFileName << " > " << tmpFileName;
		result = std::system(cmdStream.str().c_str());
		result = std::rename(tmpFileName.c_str(), origFileName.c_str());
		if (result != 0)
		{
			throw std::runtime_error("Renaming failed");
		}
	}
}

double computeSumOfDifferenceSquared(const std::vector<double>& observed, const std::vector<double>& simulated)
{
	if(observed.size() != simulated.size()) { throw std::runtime_error("size mis-match between observed and simulated values");	}
	double objFnVal = 0;
	size_t numStats = observed.size();
	for(size_t i=0; i<numStats; i++)
	{
		objFnVal = objFnVal + std::pow((simulated[i]-observed[i]), 2);
	}
	return objFnVal;
}

void aggregateStatistics(const std::vector<CalibrationStatistics>& calStatsVect, CalibrationStatistics& aggregate)
{
	for(std::vector<CalibrationStatistics>::const_iterator csIt=calStatsVect.begin();
			csIt!=calStatsVect.end(); csIt++)
	{
		aggregate = aggregate + (*csIt);
	}
}

} //end anonymous namespace

sim_mob::medium::PredayManager::PredayManager() : mtConfig(MT_Config::getInstance())
{
}

sim_mob::medium::PredayManager::~PredayManager() {
	Print() << "Clearing Person List" << std::endl;
	// clear Persons
	for(PersonList::iterator i = personList.begin(); i!=personList.end(); i++) {
		delete *i;
	}
	personList.clear();

	// clear Zones
	Print() << "Clearing zoneMap" << std::endl;
	for(ZoneMap::iterator i = zoneMap.begin(); i!=zoneMap.end(); i++) {
		delete i->second;
	}
	zoneMap.clear();

	// clear AMCosts
	Print() << "Clearing amCostMap" << std::endl;
	for(CostMap::iterator i = amCostMap.begin(); i!=amCostMap.end(); i++) {
		for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
			if(j->second) {
				delete j->second;
			}
		}
	}
	amCostMap.clear();

	// clear PMCosts
	Print() << "Clearing pmCostMap" << std::endl;
	for(CostMap::iterator i = pmCostMap.begin(); i!=pmCostMap.end(); i++) {
		for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
			if(j->second) {
				delete j->second;
			}
		}
	}
	pmCostMap.clear();

	// clear OPCosts
	Print() << "Clearing opCostMap" << std::endl;
	for(CostMap::iterator i = opCostMap.begin(); i!=opCostMap.end(); i++) {
		for(boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j!=i->second.end(); j++) {
			if(j->second) {
				delete j->second;
			}
		}
	}
	opCostMap.clear();
}

void sim_mob::medium::PredayManager::loadPersons(BackendType dbType) {
	switch(dbType) {
	case POSTGRES:
	{
		Database database = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_local_lt");
		std::string cred_id = ConfigManager::GetInstance().FullConfig().system.networkDatabase.credentials;
		Credential credentials = ConfigManager::GetInstance().FullConfig().constructs.credentials.at(cred_id);
		std::string username = credentials.getUsername();
		std::string password = credentials.getPassword(false);
		DB_Config dbConfig(database.host, database.port, database.dbName, username, password);

		// Connect to database and load data.
		DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
		conn.connect();
		if (conn.isConnected()) {
			PopulationSqlDao populationDao(conn);
			populationDao.getAll(personList);
		}
		break;
	}
	case MONGO_DB:
	{
		std::string populationCollectionName = mtConfig.getMongoCollectionsMap().getCollectionName("population");
		Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
		std::string emptyString;
		db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);
		PopulationMongoDao populationDao(dbConfig, db.dbName, populationCollectionName);
		populationDao.getAll(personList);
		break;
	}
	default:
	{
		throw std::runtime_error("Unsupported backend type. Only PostgreSQL and MongoDB are currently supported.");
	}
	}
}

void sim_mob::medium::PredayManager::loadZones(db::BackendType dbType) {
	switch(dbType) {
		case POSTGRES:
		{
			throw std::runtime_error("Zone information is not available in PostgreSQL database yet");
			break;
		}
		case MONGO_DB:
		{
			std::string zoneCollectionName = mtConfig.getMongoCollectionsMap().getCollectionName("Zone");
			Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
			std::string emptyString;
			db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);
			ZoneMongoDao zoneDao(dbConfig, db.dbName, zoneCollectionName);
			zoneDao.getAllZones(zoneMap);
			Print() << "MTZ Zones loaded" << std::endl;
			break;
		}
		default:
		{
			throw std::runtime_error("Unsupported backend type. Only PostgreSQL and MongoDB are currently supported.");
		}
	}

	for(ZoneMap::iterator i=zoneMap.begin(); i!=zoneMap.end(); i++) {
		zoneIdLookup[i->second->getZoneCode()] = i->first;
	}
}

void sim_mob::medium::PredayManager::loadZoneNodes(db::BackendType dbType) {
	switch(dbType) {
		case POSTGRES:
		{
			throw std::runtime_error("AM, PM and off peak costs are not available in PostgreSQL database yet");
		}
		case MONGO_DB:
		{
			std::string zoneNodeCollectionName = mtConfig.getMongoCollectionsMap().getCollectionName("zone_node");
			Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
			std::string emptyString;
			db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);
			ZoneNodeMappingDao zoneNodeDao(dbConfig, db.dbName, zoneNodeCollectionName);
			zoneNodeDao.getAll(zoneNodeMap);
			Print() << "Zones-Node mapping loaded" << std::endl;
			break;
		}
		default:
		{
			throw std::runtime_error("Unsupported backend type. Only PostgreSQL and MongoDB are currently supported.");
		}
	}
	Print() << "zoneNodeMap.size() : " << zoneNodeMap.size() << std::endl;
}

void sim_mob::medium::PredayManager::loadCosts(db::BackendType dbType) {
	switch(dbType) {
	case POSTGRES:
	{
		throw std::runtime_error("AM, PM and off peak costs are not available in PostgreSQL database yet");
	}
	case MONGO_DB:
	{
		const MongoCollectionsMap& mongoColl = mtConfig.getMongoCollectionsMap();
		std::string amCostsCollName = mongoColl.getCollectionName("AMCosts");
		std::string pmCostsCollName = mongoColl.getCollectionName("PMCosts");
		std::string opCostsCollName = mongoColl.getCollectionName("OPCosts");
		Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
		std::string emptyString;
		db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);

		ZoneMap::size_type nZones = zoneMap.size();
		if(nZones > 0) {
			// if the zone data was loaded already we can reserve space for costs to speed up the loading
			// Cost data will be available foe every pair (a,b) of zones where a!=b
			CostMap::size_type mapSz = nZones * nZones - nZones;
                        
                        //boost 1.49
                        amCostMap.rehash(ceil(mapSz / amCostMap.max_load_factor()));
                        pmCostMap.rehash(ceil(mapSz / pmCostMap.max_load_factor()));
                        opCostMap.rehash(ceil(mapSz / opCostMap.max_load_factor()));
			//boost >= 1.50
                        //amCostMap.reserve(mapSz);
			//pmCostMap.reserve(mapSz);
			//opCostMap.reserve(mapSz);
		}

		CostMongoDao amCostDao(dbConfig, db.dbName, amCostsCollName);
		amCostDao.getAll(amCostMap);
		Print() << "AM Costs Loaded" << std::endl;

		CostMongoDao pmCostDao(dbConfig, db.dbName, pmCostsCollName);
		pmCostDao.getAll(pmCostMap);
		Print() << "PM Costs Loaded" << std::endl;

		CostMongoDao opCostDao(dbConfig, db.dbName, opCostsCollName);
		opCostDao.getAll(opCostMap);
		Print() << "OP Costs Loaded" << std::endl;
	}
	}
}

void sim_mob::medium::PredayManager::distributeAndProcessPersons() {
	boost::thread_group threadGroup;
	unsigned numWorkers = mtConfig.getNumPredayThreads();
	if(numWorkers == 1) { // if single threaded execution was requested
		if(mtConfig.runningPredaySimulation()) {
			processPersons(personList.begin(), personList.end());
		}
		else if(mtConfig.runningPredayLogsumComputation()) {
			computeLogsums(personList.begin(), personList.end());
		}
		else if(mtConfig.runningPredayCalibration()) {
			processPersonsForCalibration(personList.begin(), personList.end(), simulatedStatsVector[0]);
		}
	}
	else {
		PersonList::size_type numPersons = personList.size();
		PersonList::size_type numPersonsPerThread = numPersons / numWorkers;
		PersonList::iterator first = personList.begin();
		PersonList::iterator last = personList.begin()+numPersonsPerThread;
		Print() << "numPersons:" << numPersons << "|numWorkers:" << numWorkers
				<< "|numPersonsPerThread:" << numPersonsPerThread << std::endl;

		/*
		 * Passing different iterators on the same list into the threaded
		 * function. So each thread will iterate mutually exclusive and
		 * exhaustive set of persons from the population.
		 *
		 * Note that each thread will iterate the same personList with different
		 * start and end iterators. It is therefore important that none of the
		 * threads change the personList.
		 */
		for(int i = 1; i<=numWorkers; i++) {
			if(mtConfig.runningPredaySimulation()) {
				threadGroup.create_thread( boost::bind(&PredayManager::processPersons, this, first, last) );
			}
			else if(mtConfig.runningPredayLogsumComputation()) {
				threadGroup.create_thread( boost::bind(&PredayManager::computeLogsums, this, first, last) );
			}
			else if(mtConfig.runningPredayCalibration()) {
				threadGroup.create_thread( boost::bind(&PredayManager::processPersonsForCalibration, this, first, last, simulatedStatsVector[i-1]) );
			}

			first = last;
			if(i+1 == numWorkers) {
				// if the next iteration is the last take all remaining persons
				last = personList.end();
			}
			else {
				last = last + numPersonsPerThread;
			}
		}
		threadGroup.join_all();
	}

}

void sim_mob::medium::PredayManager::calibratePreday()
{
	if(!mtConfig.runningPredayCalibration()) { return; }

	boost::thread_group threadGroup;
	unsigned numWorkers = mtConfig.getNumPredayThreads();
	loadCalibrationVariables();

	const PredayCalibrationParams& predayParams = mtConfig.getPredayCalibrationParams();
	{
		CalibrationStatistics observedStats(predayParams.getObservedStatisticsFile());
		observedStats.getAllStatistics(observedHITS_Stats); // store a local copy for future use
	} //scope out observedStats

	loadWeightMatrix();

	simulatedStatsVector = std::vector<CalibrationStatistics>(mtConfig.getNumPredayThreads());
	unsigned iterationLimit = predayParams.getIterationLimit();
	bool consoleOutput = mtConfig.isConsoleOutput();
	const MongoCollectionsMap& mongoColl = mtConfig.getMongoCollectionsMap();
	Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
	std::string emptyString;
	const std::map<std::string, std::string>& collectionNameMap = mongoColl.getCollectionsMap();
	for(std::map<std::string, std::string>::const_iterator i=collectionNameMap.begin(); i!=collectionNameMap.end(); i++) {
		db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);
		mongoDao[i->first]= new db::MongoDao(dbConfig, db.dbName, i->second);
	}

	RandomSymmetricPlusMinusVector randomVector(calibrationVariablesList.size());
	double initialGradientStepSize = 0, stepSize = 0;

	objectiveFunctionValues.clear();
	//iteration 0
	computeObjectiveFunction(calibrationVariablesList);

	for(unsigned k=1; k<=iterationLimit; k++)
	{
		// iteration k
		// 1. compute perturbation step size
		initialGradientStepSize = predayParams.getInitialGradientStepSize() / std::pow((1+k), predayParams.getAlgorithmCoefficient2());

		// 2. compute gradients using SPSA technique
		std::vector<double> gradientVector;
		if(mtConfig.runningWSPSA())
		{
			computeWeightedGradient(randomVector.get(), initialGradientStepSize, gradientVector);
		}
		else
		{
			computeGradient(randomVector.get(), initialGradientStepSize, gradientVector);
		}


		// 3. compute projected anti-gradient vector
		std::vector<double> antiGradientVector;
		computeAntiGradientVector(gradientVector, antiGradientVector);

		// 4. compute step size
		stepSize = predayParams.getInitialStepSize() / std::pow((predayParams.getInitialStepSize()+k+predayParams.getStabilityConstant()), predayParams.getAlgorithmCoefficient1());

		// 5. update the variables being calibrated and compute objective function
		std::vector<double> scaledAntiGradientVector;
		multiplyScalarWithVector(stepSize, antiGradientVector, scaledAntiGradientVector);
		updateVariablesByAddition(calibrationVariablesList, scaledAntiGradientVector);
		double objFn = computeObjectiveFunction(calibrationVariablesList);
		objectiveFunctionValues.push_back(objFn);

		// 6. check termination. At this point there would be atleast k+1 elements in objectiveFunctionValues
		if(std::abs(objectiveFunctionValues[k] - objectiveFunctionValues[k-1]) <= predayParams.getTolerance())
		{
			//converged!
			break;
		}
	}

	// destroy Dao objects
	for(boost::unordered_map<std::string, db::MongoDao*>::iterator i=mongoDao.begin(); i!=mongoDao.end(); i++) {
		safe_delete_item(i->second);
	}
	mongoDao.clear();

}

void sim_mob::medium::PredayManager::loadCalibrationVariables()
{
	CSV_Reader variablesReader(mtConfig.getPredayCalibrationParams().getCalibrationVariablesFile(), true);
	boost::unordered_map<std::string, std::string> variableRow;
	variablesReader.getNextRow(variableRow, false);
	while (!variableRow.empty())
	{
		try
		{
			CalibrationVariable calibrationVariable;
			calibrationVariable.setVariableName(variableRow.at(VARIABLE_NAME_CSV_HEADER));
			calibrationVariable.setScriptFileName(variableRow.at(LUA_FILE_NAME_CSV_HEADER));
			calibrationVariable.setInitialValue(boost::lexical_cast<double>(variableRow.at(INITIAL_VALUE_CSV_HEADER)));
			calibrationVariable.setCurrentValue(boost::lexical_cast<double>(variableRow.at(INITIAL_VALUE_CSV_HEADER)));
			calibrationVariable.setLowerLimit(boost::lexical_cast<double>(variableRow.at(LOWER_LIMIT_CSV_HEADER)));
			calibrationVariable.setUpperLimit(boost::lexical_cast<double>(variableRow.at(UPPER_LIMIT_CSV_HEADER)));
			calibrationVariablesList.push_back(calibrationVariable);
		}
		catch(const std::out_of_range& oor)
		{
			throw std::runtime_error("Header mis-match while reading calibration variables csv");
		}
		catch(boost::bad_lexical_cast const&)
		{
			throw std::runtime_error("Invalid value found in calibration variables csv");
		}

		variableRow.clear();
		variablesReader.getNextRow(variableRow, false);
	}
}

void sim_mob::medium::PredayManager::loadWeightMatrix()
{
	CSV_Reader matrixReader(mtConfig.getWSPSA_CalibrationParams().getWeightMatrixFile(), true);
	size_t numVariables = calibrationVariablesList.size();
	size_t numStatistics = observedHITS_Stats.size();
	weightMatrix = matrix<double>(numStatistics, numVariables);
	std::vector<std::string> matrixRow;
	matrixReader.getNextRow(matrixRow);
	size_t row=0, col=0;
	while(!matrixRow.empty())
	{
		std::vector<std::string>::iterator strIt=matrixRow.begin();
		strIt++; //first element is the name of the observation
		for(; strIt!=matrixRow.end(); strIt++)
		{
			try
			{
				weightMatrix(row,col) = boost::lexical_cast<double>(*strIt);
			}
			catch(boost::bad_lexical_cast const& badCastEx)
			{
				throw std::runtime_error("Invalid value found in weight matrix csv");
			}
			catch(bad_index const& badIdxEx)
			{
				throw std::runtime_error("weight matrix csv has incorrect number of elements");
			}
			col++;
		}
		row++;
	}
}

void sim_mob::medium::PredayManager::processPersonsForCalibration(PersonList::iterator firstPersonIt, PersonList::iterator oneAfterLastPersonIt, CalibrationStatistics& simStats)
{
	bool consoleOutput = mtConfig.isConsoleOutput();
	for(PersonList::iterator i = firstPersonIt; i!=oneAfterLastPersonIt; i++) {
		PredaySystem predaySystem(**i, zoneMap, zoneIdLookup, amCostMap, pmCostMap, opCostMap, mongoDao);
		predaySystem.planDay();
		predaySystem.updateStatistics(simStats);
		if(consoleOutput) { predaySystem.printLogs(); }
	}
}

void sim_mob::medium::PredayManager::computeGradient(const std::vector<short>& randomVector, double initialGradientStepSize,
		std::vector<double>& gradientVector)
{
	size_t numVariables = calibrationVariablesList.size();
	if(randomVector.size() != calibrationVariablesList.size()) {throw std::runtime_error("size mis-match between calibrationVariablesList and randomVector"); }
	std::vector<double> scaledRandomVector;
	multiplyScalarWithVector(initialGradientStepSize, randomVector, scaledRandomVector);
	std::vector<CalibrationVariable> localCopyCalVarList = calibrationVariablesList;
	updateVariablesByAddition(localCopyCalVarList, scaledRandomVector);
	double objFnPlus = computeObjectiveFunction(localCopyCalVarList);
	localCopyCalVarList = calibrationVariablesList;
	updateVariablesBySubtraction(localCopyCalVarList, scaledRandomVector);
	double objFnMinus = computeObjectiveFunction(localCopyCalVarList);

	gradientVector.clear();
	double varGradient = 0;
	for(size_t i=0; i<numVariables; i++)
	{
		varGradient = (objFnPlus - objFnMinus)/(2*scaledRandomVector[i]);
		gradientVector.push_back(varGradient);
	}
}

void sim_mob::medium::PredayManager::computeWeightedGradient(const std::vector<short>& randomVector, double initialGradientStepSize,
		std::vector<double>& gradientVector)
{
	size_t numVariables = calibrationVariablesList.size();
	size_t numStatistics = observedHITS_Stats.size();

	if(randomVector.size() != calibrationVariablesList.size()) {throw std::runtime_error("size mis-match between calibrationVariablesList and randomVector"); }

	std::vector<double> scaledRandomVector;
	multiplyScalarWithVector(initialGradientStepSize, randomVector, scaledRandomVector);

	std::vector<double> simStatisticsPlus, simStatisticsMinus;

	std::vector<CalibrationVariable> localCopyCalVarList = calibrationVariablesList;
	updateVariablesByAddition(localCopyCalVarList, scaledRandomVector);
	computeObservationsVector(localCopyCalVarList, simStatisticsPlus);
	if(observedHITS_Stats.size() != simStatisticsPlus.size()) { throw std::runtime_error("size mis-match between observed and simulated values");	}

	localCopyCalVarList = calibrationVariablesList;
	updateVariablesBySubtraction(localCopyCalVarList, scaledRandomVector);
	computeObservationsVector(localCopyCalVarList, simStatisticsMinus);
	if(observedHITS_Stats.size() != simStatisticsMinus.size()) { throw std::runtime_error("size mis-match between observed and simulated values");	}

	gradientVector.clear();
	double gradient = 0, squaredDifferencePlus = 0, squaredDifferenceMinus = 0;
	for(size_t j=0; j<numVariables; j++)
	{
		gradient = 0;
		for(size_t i=0; i<numStatistics; i++)
		{
			squaredDifferencePlus = std::pow((simStatisticsPlus[i]-observedHITS_Stats[i]), 2);
			squaredDifferenceMinus = std::pow((simStatisticsMinus[i]-observedHITS_Stats[i]), 2);
			gradient = gradient + (weightMatrix(i,j) * (squaredDifferencePlus - squaredDifferenceMinus));
		}
		gradient = gradient / (2*scaledRandomVector[j]);
		gradientVector.push_back(gradient);
	}
}

void sim_mob::medium::PredayManager::processPersons(PersonList::iterator firstPersonIt, PersonList::iterator oneAfterLastPersonIt)
{
	boost::unordered_map<std::string, db::MongoDao*> mongoDao;
	bool outputTripchains = mtConfig.isOutputTripchains();
	bool consoleOutput = mtConfig.isConsoleOutput();
	const MongoCollectionsMap& mongoColl = mtConfig.getMongoCollectionsMap();
	Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
	std::string emptyString;
	const std::map<std::string, std::string>& collectionNameMap = mongoColl.getCollectionsMap();
	for(std::map<std::string, std::string>::const_iterator i=collectionNameMap.begin(); i!=collectionNameMap.end(); i++) {
		db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);
		mongoDao[i->first]= new db::MongoDao(dbConfig, db.dbName, i->second);
	}

	Database database = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_local");
	std::string cred_id = ConfigManager::GetInstance().FullConfig().system.networkDatabase.credentials;
	Credential credentials = ConfigManager::GetInstance().FullConfig().constructs.credentials.at(cred_id);
	std::string username = credentials.getUsername();
	std::string password = credentials.getPassword(false);
	DB_Config dbConfig(database.host, database.port, database.dbName, username, password);

	// Connect to database and load data.
	DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
	if(outputTripchains)
	{
		conn.connect();
		if (!conn.isConnected()) {
			throw std::runtime_error("Could not connect to PostgreSQL database. Check credentials.");
		}
	}
	TripChainSqlDao tcDao(conn);

	// loop through all persons within the range and plan their day
	for(PersonList::iterator i = firstPersonIt; i!=oneAfterLastPersonIt; i++) {
		PredaySystem predaySystem(**i, zoneMap, zoneIdLookup, amCostMap, pmCostMap, opCostMap, mongoDao);
		predaySystem.planDay();
		predaySystem.outputPredictionsToMongo();
		if(outputTripchains) { predaySystem.outputTripChainsToPostgreSQL(zoneNodeMap, tcDao); }
		if(consoleOutput) { predaySystem.printLogs(); }
	}

	// destroy Dao objects
	for(boost::unordered_map<std::string, db::MongoDao*>::iterator i=mongoDao.begin(); i!=mongoDao.end(); i++) {
		safe_delete_item(i->second);
	}
	mongoDao.clear();
}

void sim_mob::medium::PredayManager::computeLogsums(PersonList::iterator firstPersonIt, PersonList::iterator oneAfterLastPersonIt)
{
	boost::unordered_map<std::string, db::MongoDao*> mongoDao;
	bool consoleOutput = mtConfig.isConsoleOutput();
	const MongoCollectionsMap& mongoColl = mtConfig.getMongoCollectionsMap();
	Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
	std::string emptyString;
	const std::map<std::string, std::string>& collectionNameMap = mongoColl.getCollectionsMap();
	for(std::map<std::string, std::string>::const_iterator i=collectionNameMap.begin(); i!=collectionNameMap.end(); i++) {
		db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);
		mongoDao[i->first]= new db::MongoDao(dbConfig, db.dbName, i->second);
	}

	// loop through all persons within the range and plan their day
	for(PersonList::iterator i = firstPersonIt; i!=oneAfterLastPersonIt; i++) {
		PredaySystem predaySystem(**i, zoneMap, zoneIdLookup, amCostMap, pmCostMap, opCostMap, mongoDao);
		predaySystem.computeLogsums();
		predaySystem.outputLogsumsToMongo();
		if(consoleOutput) { predaySystem.printLogs(); }
	}

	// destroy Dao objects
	for(boost::unordered_map<std::string, db::MongoDao*>::iterator i=mongoDao.begin(); i!=mongoDao.end(); i++) {
		safe_delete_item(i->second);
	}
	mongoDao.clear();
}

double sim_mob::medium::PredayManager::computeObjectiveFunction(const std::vector<CalibrationVariable>& calVarList)
{
	std::vector<double> simulatedHITS_Stats;
	computeObservationsVector(calVarList, simulatedHITS_Stats);
	return computeSumOfDifferenceSquared(observedHITS_Stats, simulatedHITS_Stats);
}

void sim_mob::medium::PredayManager::computeObservationsVector(const std::vector<CalibrationVariable>& calVarList, std::vector<double> simulatedHITS_Stats)
{
	updateVariablesInLuaFiles(mtConfig.getModelScriptsMap().getPath(), calVarList);
	std::for_each(simulatedStatsVector.begin(), simulatedStatsVector.end(), std::mem_fun_ref(&CalibrationStatistics::reset));
	distributeAndProcessPersons();
	simulatedHITS_Stats.clear();
	CalibrationStatistics simulatedStats;
	aggregateStatistics(simulatedStatsVector, simulatedStats);
	simulatedStats.getAllStatistics(simulatedHITS_Stats);
}
