//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PredayManager.hpp"

#include <algorithm>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <list>
#include <map>
#include <string>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Connection.hpp"
#include "database/DB_Config.hpp"
#include "database/predaydao/DatabaseHelper.hpp"
#include "database/predaydao/PopulationSqlDao.hpp"
#include "database/predaydao/ZoneCostSqlDao.hpp"
#include "logging/NullableOutputStream.hpp"
#include "logging/Log.hpp"
#include "util/CSVReader.hpp"
#include "util/LangHelpers.hpp"
#include "util/Utils.hpp"

using namespace std;
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

//initialize once and use multiple times
std::vector<double> observedHITS_Stats;
std::vector<double> statsScale;

matrix<double> weightMatrix;

/**keep a file wise list of variables to calibrate*/
boost::unordered_map<std::string, std::vector<std::string> > variablesInFileMap;

template<typename V>
void printVector(const std::string vecName, const std::vector<V>& vec)
{
	std::stringstream ss;
	ss << vecName << " - [";
	for (typename std::vector<V>::const_iterator vIt = vec.begin(); vIt != vec.end(); vIt++)
	{
		ss << "," << (*vIt);
	}
	ss << "]\n";
	Print() << ss.str();
}

/**
 * streams the elements of vector into stringstream as comma seperated values
 * @param vectorToLog input vector
 * @param logStream stringstream to write to
 */
template<typename V>
void streamVector(const std::vector<V>& vectorToLog, std::stringstream& logStream)
{
	for (typename std::vector<V>::const_iterator vIt = vectorToLog.begin(); vIt != vectorToLog.end(); vIt++)
	{
		logStream << "," << *vIt;
	}
}

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
	RandomSymmetricPlusMinusVector(size_t dimension) :
			dimension(dimension)
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
		for (size_t i = 0; i < dimension; i++)
		{
			if ((rand() % 2) == 0)
			{
				randomVector.push_back(1);
			}
			else
			{
				randomVector.push_back(-1);
			}
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

void populateScalesVector(const std::vector<std::string>& scalesVector)
{
	size_t numStats = observedHITS_Stats.size();
	if (scalesVector.empty())
	{
		statsScale = std::vector<double>(numStats, 1); // initialize the vector with 1 as scale for all stats
	}
	else
	{
		if (scalesVector.size() != numStats)
		{
			throw std::runtime_error("scale not provided for all statistics. Check observed statistics CSV file");
		}
		try
		{
			for (std::vector<std::string>::const_iterator sclIt = scalesVector.begin(); sclIt != scalesVector.end(); sclIt++)
			{
				statsScale.push_back(boost::lexical_cast<double>(*sclIt));
			}
		}
		catch (boost::bad_lexical_cast const& badCast)
		{
			throw std::runtime_error("observed statistics file has non-numeric values for scale");
		}
	}
}

/**
 * helper function to compute Anti gradient
 * @param gradientVector the gradient vector
 */
void computeAntiGradient(std::vector<double>& gradientVector)
{
	if (gradientVector.size() != calibrationVariablesList.size())
	{
		throw std::runtime_error("gradientVector.size() != calibrationVariablesList.size()");
	}
	double antiGradient = 0;
	size_t variableIdx = 0;
	for (std::vector<double>::iterator gradIt = gradientVector.begin(); gradIt != gradientVector.end(); gradIt++, variableIdx++)
	{
		double& gradient = *gradIt;
		const CalibrationVariable& calVar = calibrationVariablesList[variableIdx];
		if ((gradient > 0 && calVar.getLowerLimit() == calVar.getCurrentValue()) || (gradient < 0 && calVar.getUpperLimit() == calVar.getCurrentValue()))
		{
			gradient = 0;
		}
		else
		{
			gradient = -gradient;
		}
	}
}

/**
 * helper function to multiply a scalar and a vector
 * @param scalarLhs the scalar
 * @param vectorRhs the vector
 * @param output output vector to populate with scaled values
 */
template<typename V>
void multiplyScalarWithVector(double scalarLhs, const std::vector<V>& vectorRhs, std::vector<double>& output)
{
	for (typename std::vector<V>::const_iterator vIt = vectorRhs.begin(); vIt != vectorRhs.end(); vIt++)
	{
		double result = (*vIt) * scalarLhs;
		output.push_back(result);
	}
}

/**
 * updates the current values of variables by adding gradients
 * @param calVarList variable list
 * @param gradientVector the gradient vector
 * @param scale the scaling factor for gradients
 */
template<typename V>
void updateVariablesByAddition(std::vector<CalibrationVariable>& calVarList, const std::vector<V>& gradientVector, double scale = 1)
{
	if (calVarList.size() != gradientVector.size())
	{
		throw std::runtime_error("cannot add vectors of different sizes");
	}
	size_t i = 0;
	double newVal = 0;
	for (std::vector<CalibrationVariable>::iterator calVarIt = calVarList.begin(); calVarIt != calVarList.end(); calVarIt++, i++)
	{
		CalibrationVariable& calVar = (*calVarIt);
		newVal = calVar.getCurrentValue() - (scale * gradientVector[i]);
		if (newVal > calVar.getUpperLimit())
		{
			calVar.setCurrentValue(calVar.getUpperLimit());
		}
		else if (newVal < calVar.getLowerLimit())
		{
			calVar.setCurrentValue(calVar.getLowerLimit());
		}
		else
		{
			calVar.setCurrentValue(newVal);
		}
	}
}

/**
 * updates the current values of variables by subtracting gradients
 * @param calVarList variable list
 * @param gradientVector the gradient vector
 * @param scale the scaling factor for gradients
 */
template<typename V>
void updateVariablesBySubtraction(std::vector<CalibrationVariable>& calVarList, const std::vector<V>& gradientVector, double scale = 1)
{
	if (calVarList.size() != gradientVector.size())
	{
		throw std::runtime_error("cannot subtract vectors of different sizes");
	}
	size_t i = 0;
	double newVal = 0;
	for (std::vector<CalibrationVariable>::iterator calVarIt = calVarList.begin(); calVarIt != calVarList.end(); calVarIt++, i++)
	{
		CalibrationVariable& calVar = (*calVarIt);
		newVal = calVar.getCurrentValue() - (scale * gradientVector[i]);
		if (newVal < calVar.getLowerLimit())
		{
			calVar.setCurrentValue(calVar.getLowerLimit());
		}
		else if (newVal > calVar.getUpperLimit())
		{
			calVar.setCurrentValue(calVar.getUpperLimit());
		}
		else
		{
			calVar.setCurrentValue(newVal);
		}
	}
}

void constructVariablesInFileMap(const std::vector<CalibrationVariable>& calVarList)
{
	for (std::vector<CalibrationVariable>::const_iterator calVarIt = calVarList.begin(); calVarIt != calVarList.end(); calVarIt++)
	{
		const CalibrationVariable& calVar = *calVarIt;
		variablesInFileMap[calVar.getScriptFileName()].push_back(calVar.getVariableName());
	}
}

/**
 * updates the values of parameters in lua file
 * \note this function executes std::system() to call sed script if command processor is available
 * @param scriptFilesPath path to scripts
 * @param calVarList list of variables to calibrate
 */
void updateVariablesInLuaFiles(const std::string& scriptFilesPath, const std::vector<CalibrationVariable>& calVarList)
{
	int result = 0;
	if (std::system(nullptr)) // If command processor is available, we can simply use sed to search and replace in files
	{
		std::stringstream cmdStream;
		for (std::vector<CalibrationVariable>::const_iterator calVarIt = calVarList.begin(); calVarIt != calVarList.end(); calVarIt++)
		{
			const CalibrationVariable& calVar = *calVarIt;
			const std::string origFileName(scriptFilesPath + calVar.getScriptFileName());
			const std::string tmpFileName(origFileName + ".tmp");
			cmdStream.str(std::string());
			cmdStream << "sed -e 's/local " << calVar.getVariableName() << ".*=.*[0-9]/local " << calVar.getVariableName() << " = " << calVar.getCurrentValue()
					<< "/g' " << origFileName << " > " << tmpFileName;
			result = std::system(cmdStream.str().c_str());
			result = std::rename(tmpFileName.c_str(), origFileName.c_str());
			if (result)
			{
				std::stringstream err;
				err << "Renaming failed for file " << origFileName << ": error - " << std::strerror(errno) << std::endl;
				throw std::runtime_error(err.str());
			}
		}
	}
	else // we need to do it the hard way
	{
		// construct variable value map
		boost::unordered_map<std::string, double> variableValueMap;
		for (std::vector<CalibrationVariable>::const_iterator calVarIt = calVarList.begin(); calVarIt != calVarList.end(); calVarIt++)
		{
			const CalibrationVariable& calVar = *calVarIt;
			variableValueMap[calVar.getVariableName()] = calVar.getCurrentValue();
		}

		//iterate all lua files to check for variables to change
		for (boost::unordered_map<std::string, std::vector<std::string> >::iterator i = variablesInFileMap.begin(); i != variablesInFileMap.end(); i++)
		{
			const std::string origFileName(scriptFilesPath + i->first);
			const std::string tmpFileName(origFileName + ".tmp");

			//open the lua file
			ifstream in(origFileName.c_str());
			if (!in.is_open())
			{
				std::stringstream err;
				err << "Could not open file: " << origFileName;
				throw std::runtime_error(err.str());
			}

			//open the tmp output file
			ofstream out(tmpFileName.c_str());

			//loop through each line and search for patterns. update new value if pattern is matched
			std::string line;
			std::string assignmentLhs;
			while (std::getline(in, line))
			{
				size_t pos;
				pos = line.find("local ");
				if (pos == std::string::npos)
				{
					out << line << "\n";
					continue;
				} // quickly skip irrelevant lines

				const std::vector<std::string>& varNames = i->second;
				bool updated = false;
				for (std::vector<std::string>::const_iterator strIt = varNames.begin(); strIt != varNames.end(); strIt++)
				{
					const std::string& varName = *strIt;
					pos = line.find("local " + varName);
					if (pos != std::string::npos) //found
					{
						pos = line.find("=");
						if (pos != std::string::npos)
						{
							std::stringstream updatedLine;
							updatedLine << line.substr(0, pos) << "= " << variableValueMap.at(varName);
							out << updatedLine.str() << "\n";
							updated = true;
							break; //there can only be one variable declaration in one line in lua
						}
					}
				}
				if (!updated)
				{
					out << line << "\n";
				}
			}

			in.close();
			out.close();
			// delete the original file
			result = std::remove(origFileName.c_str());
			if (result)
			{
				std::stringstream err;
				err << "File deletion failed for file " << origFileName << ": error - " << std::strerror(errno) << std::endl;
				throw std::runtime_error(err.str());
			}
			// rename tmp file to original file name
			result = std::rename(tmpFileName.c_str(), origFileName.c_str());
			if (result)
			{
				std::stringstream err;
				err << "Renaming failed for file " << origFileName << ": error - " << std::strerror(errno) << std::endl;
				throw std::runtime_error(err.str());
			}
		}
	}
}

/**
 * computes sum of difference squared of simulated and observed vectors
 * @param observed observed values
 * @param simulated simulated values
 */
double computeSumOfDifferenceSquared(const std::vector<double>& observed, const std::vector<double>& simulated)
{
	if (observed.size() != simulated.size())
	{
		throw std::runtime_error("size mis-match between observed and simulated values");
	}
	double objFnVal = 0, scaledDiff = 0;
	size_t numStats = observed.size();
	for (size_t i = 0; i < numStats; i++)
	{
		scaledDiff = statsScale[i] * (simulated[i] - observed[i]);
		objFnVal = objFnVal + std::pow(scaledDiff, 2);
	}
	return objFnVal;
}

/**
 * adds up CalibrationStatistics vector
 * @param calStatsVect input vector
 * @param aggregate reference to output of addition
 */
void aggregateStatistics(const std::vector<CalibrationStatistics>& calStatsVect, CalibrationStatistics& aggregate)
{
	for (std::vector<CalibrationStatistics>::const_iterator csIt = calStatsVect.begin(); csIt != calStatsVect.end(); csIt++)
	{
		aggregate = aggregate + (*csIt);
	}
}

/**
 * operator overload to simplify logging to file
 * @param os the lhs of the operator
 * @param calVar the rhs of the operator
 * @return a reference to os after adding calVar to it
 */
std::ostream& operator <<(std::ostream& os, const CalibrationVariable& calVar)
{
	os << calVar.getCurrentValue();
	return os;
}

/**
 * Pretty printer for calibration variable list
 * @param calVarList vector of variables to print
 * @param simType type of simulation
 */
void printParameters(const std::vector<CalibrationVariable>& calVarList, const std::string& simType)
{
	std::stringstream ss;
	ss << "Values for " << simType << std::endl;
	for (std::vector<CalibrationVariable>::const_iterator i = calVarList.begin(); i != calVarList.end(); i++)
	{
		const CalibrationVariable& calVar = *i;
		ss << calVar.getScriptFileName() << " - " << calVar.getVariableName() << " = " << calVar.getCurrentValue() << std::endl;
	}
	Print() << ss.str();
}

/**
 * flush stream contents to file
 * @param logHandle handle to output file
 * @param strm stringstream to flush
 */
void outputToFile(std::ofstream& logHandle, std::stringstream& strm)
{
	NullableOutputStream(&logHandle) << strm.str() << std::flush;
	strm.str(std::string());
}

/**
 * constructs unique file name strings by appending serial numbers to prefix
 * @param numFiles number of file names to generate
 * @param prefix prefix string to use in file
 * @param outFileNamesList out list to populate with generated file names
 */
void constructFileNames(size_t numFiles, const std::string& prefix, std::list<std::string>& outFileNamesList)
{
	std::stringstream fileName;
	for (unsigned i = 1; i <= numFiles; i++)
	{
		fileName << prefix << i << ".log";
		outFileNamesList.push_back(fileName.str());
		fileName.str(std::string());
	}
}

void mergeCSV_Files(const std::list<std::string>& fileNameList, const std::string& fileName)
{
	//This can take some time.
	StopWatch sw;
	sw.start();
	std::cout << "Merging files, this can take several minutes...\n";

	//One-by-one.
	std::ofstream out(fileName.c_str(), std::ios::trunc | std::ios::binary);
	if (!out.good())
	{
		throw std::runtime_error("Error: Can't write to file.");
	}
	for (std::list<std::string>::const_iterator it = fileNameList.begin(); it != fileNameList.end(); it++)
	{
		Print() << "  Merging: " << *it << std::endl;
		std::ifstream src(it->c_str(), std::ios::binary);
		if (src.fail())
		{
			throw std::runtime_error("Error: Can't read from file.");
		}

		//If src is good, this part's easy.
		out << src.rdbuf();
		src.close();
	}
	out.close();

	sw.stop();
	std::cout << "Files merged; took " << sw.getTime() << "s\n";
}

/**
 * constructs and returns a DB_Connection object
 * @param dbInfo object holding the ids of configurations to construct the connection string
 * @return DB_Connection object
 */
DB_Connection getDB_Connection(const DatabaseDetails& dbInfo)
{
	const std::string& dbId = dbInfo.database;
	const std::string& cred_id = dbInfo.credentials;
	const ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();
	Database db = cfg.constructs.databases.at(dbId);
	Credential cred = cfg.constructs.credentials.at(cred_id);
	std::string username = cred.getUsername();
	std::string password = cred.getPassword(false);
	DB_Config dbConfig(db.host, db.port, db.dbName, username, password);
	return DB_Connection(sim_mob::db::POSTGRES, dbConfig);
}

} //end anonymous namespace

sim_mob::medium::PredayManager::PredayManager() :
		mtConfig(MT_Config::getInstance()), logFile(nullptr)
{
}

sim_mob::medium::PredayManager::~PredayManager()
{
	Print() << "Clearing Person List\n";
	// clear Persons
	for (PersonList::iterator i = personList.begin(); i != personList.end(); i++)
	{
		delete *i;
	}
	personList.clear();

	// clear Zones
	Print() << "Clearing zoneMap\n";
	for (ZoneMap::iterator i = zoneMap.begin(); i != zoneMap.end(); i++)
	{
		delete i->second;
	}
	zoneMap.clear();

	// clear AMCosts
	Print() << "Clearing amCostMap\n";
	for (CostMap::iterator i = amCostMap.begin(); i != amCostMap.end(); i++)
	{
		for (boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j != i->second.end(); j++)
		{
			if (j->second)
			{
				delete j->second;
			}
		}
	}
	amCostMap.clear();

	// clear PMCosts
	Print() << "Clearing pmCostMap\n";
	for (CostMap::iterator i = pmCostMap.begin(); i != pmCostMap.end(); i++)
	{
		for (boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j != i->second.end(); j++)
		{
			if (j->second)
			{
				delete j->second;
			}
		}
	}
	pmCostMap.clear();

	// clear OPCosts
	Print() << "Clearing opCostMap\n";
	for (CostMap::iterator i = opCostMap.begin(); i != opCostMap.end(); i++)
	{
		for (boost::unordered_map<int, CostParams*>::iterator j = i->second.begin(); j != i->second.end(); j++)
		{
			if (j->second)
			{
				delete j->second;
			}
		}
	}
	opCostMap.clear();

	// clear Zone node map
	Print() << "Clearing zoneNodeMap\n";
	for (ZoneNodeMap::iterator znNdMapIt = zoneNodeMap.begin(); znNdMapIt != zoneNodeMap.end(); znNdMapIt++)
	{
		std::vector<ZoneNodeParams*>& znParamsList = znNdMapIt->second;
		for (std::vector<ZoneNodeParams*>::iterator znNdPrmsIt = znParamsList.begin(); znNdPrmsIt != znParamsList.end(); znNdPrmsIt++)
		{
			safe_delete_item(*znNdPrmsIt);
		}
		znParamsList.clear();
	}
	zoneNodeMap.clear();
}

void sim_mob::medium::PredayManager::loadPersonIds()
{
	// population data source
	DB_Connection conn = getDB_Connection(ConfigManager::GetInstance().FullConfig().populationDatabase);
	conn.connect();
	if (conn.isConnected())
	{
		PopulationSqlDao populationDao(conn);
		populationDao.getIncomeCategories(PersonParams::getIncomeCategoryLowerLimits());
		populationDao.getAddresses(PersonParams::getAddressLookup(), PersonParams::getZoneAddresses());
		populationDao.getAllIds(ltPersonIdList);
	}
}

void sim_mob::medium::PredayManager::loadZones()
{
	DB_Connection simmobConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().networkDatabase);
	simmobConn.connect();
	if (simmobConn.isConnected())
	{
		ZoneSqlDao zoneDao(simmobConn);
		zoneDao.getAll(zoneMap, &ZoneParams::getZoneId);
		Print() << "MTZ Zones loaded\n";
	}
	else
	{
		throw std::runtime_error("connection failure. Could not MTZs zones");
	}

	for (auto& zoneMapKeyVal : zoneMap)
	{
		zoneIdLookup[zoneMapKeyVal.second->getZoneCode()] = zoneMapKeyVal.first;
	}
}

void sim_mob::medium::PredayManager::loadZoneNodes()
{
	DB_Connection simmobConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().networkDatabase);
	simmobConn.connect();
	if (simmobConn.isConnected())
	{
		ZoneNodeSqlDao zoneNodeDao(simmobConn);
		zoneNodeDao.getZoneNodeMap(zoneNodeMap);
		Print() << "Zones-Node mapping loaded\n";
	}
	else
	{
		throw std::runtime_error("simmob db connection failure!");
	}
}

void sim_mob::medium::PredayManager::loadPostcodeNodeMapping()
{
	DB_Connection simmobConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().networkDatabase);
	simmobConn.connect();
	if (simmobConn.isConnected())
	{
		SimmobSqlDao simmobSqlDao(simmobConn, "");
		simmobSqlDao.getPostcodeNodeMap(PersonParams::getPostcodeNodeMap());
	}
	else
	{
		throw std::runtime_error("simmob db connection failure!");
	}
}

void sim_mob::medium::PredayManager::loadCosts()
{
	ZoneMap::size_type nZones = zoneMap.size();
	if (nZones > 0)
	{
		// if the zone data was loaded already we can reserve space for costs to speed up the loading
		// Cost data will be available for every pair (a,b) of zones where a!=b
		CostMap::size_type mapSz = nZones * nZones - nZones;

		amCostMap.rehash(ceil(mapSz / amCostMap.max_load_factor()));
		pmCostMap.rehash(ceil(mapSz / pmCostMap.max_load_factor()));
		opCostMap.rehash(ceil(mapSz / opCostMap.max_load_factor()));
	}

	DB_Connection simmobConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().networkDatabase);
	simmobConn.connect();
	if (simmobConn.isConnected())
	{
		CostSqlDao amCostDao(simmobConn, DB_GET_ALL_AM_COSTS);
		amCostDao.getAll(amCostMap);
		Print() << "AM costs loaded\n";

		CostSqlDao pmCostDao(simmobConn, DB_GET_ALL_PM_COSTS);
		pmCostDao.getAll(pmCostMap);
		Print() << "PM costs loaded\n";

		CostSqlDao opCostDao(simmobConn, DB_GET_ALL_OP_COSTS);
		opCostDao.getAll(opCostMap);
		Print() << "OP costs loaded\n";
	}
	else
	{
		throw std::runtime_error("simmob db connection failure!");
	}
}

void sim_mob::medium::PredayManager::loadUnavailableODs()
{
	DB_Connection simmobConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().networkDatabase);
	simmobConn.connect();
	if (simmobConn.isConnected())
	{
		TimeDependentTT_SqlDao tcostDao(simmobConn);
		tcostDao.getUnavailableODs(TravelTimeMode::TT_PUBLIC, unavailableODs);
		tcostDao.getUnavailableODs(TravelTimeMode::TT_PRIVATE, unavailableODs); // this can create duplicates (already inserted OD pairs) to be inserted. But that's okay!
		std::sort(unavailableODs.begin(), unavailableODs.end()); // so that future lookups can be O(log n)
		Print() << "Unavailable ODs loaded\n";
	}
}

void sim_mob::medium::PredayManager::dispatchLT_Persons()
{
	boost::thread_group threadGroup;
	unsigned numWorkers = mtConfig.getNumPredayThreads();
	std::list<std::string> logFileNames;
	std::string logFileNamePrefix;
	if (mtConfig.runningPredaySimulation())
	{
		logFileNamePrefix = "activity_schedule";
	}
	constructFileNames(numWorkers, logFileNamePrefix, logFileNames);

	if(mtConfig.runningPredayLogsumComputation())
	{
		// logsum data source
		DB_Connection simmobConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().networkDatabase);
		simmobConn.connect();
		if (!simmobConn.isConnected())
		{
			throw std::runtime_error("simmobility db connection failure!");
		}
		const std::string& logsumTableName = mtConfig.getLogsumTableName();
		SimmobSqlDao logsumSqlDao(simmobConn, logsumTableName);
		bool truncated = logsumSqlDao.erase(db::EMPTY_PARAMS);
		if(truncated)
		{
			Print() << logsumTableName << " truncated\n";
		}
		else
		{
			Print() << logsumTableName << " truncation failed!\n";
		}
	}

	if (numWorkers == 1)
	{ // if single threaded execution was requested
		if (mtConfig.runningPredaySimulation())
		{
			processPersonsForLT_Population(ltPersonIdList.begin(), ltPersonIdList.end(), logFileNames.front());
		}
		else if (mtConfig.runningPredayLogsumComputation())
		{
			computeLogsumsForLT_Population(ltPersonIdList.begin(), ltPersonIdList.end());
		}
	}
	else
	{
		LT_PersonIdList::size_type numPersons = ltPersonIdList.size();
		LT_PersonIdList::size_type numPersonsPerThread = numPersons / numWorkers;
		Print() << "numPersons:" << numPersons << "|numWorkers:" << numWorkers << "|numPersonsPerThread:" << numPersonsPerThread << std::endl;

		/*
		 * We are passing different iterators on the same list into the threaded
		 * function. Each thread will iterate through a mutually exclusive and
		 * exhaustive set of persons from the population.
		 *
		 * Note that each thread will iterate the same personList with different
		 * start and end iterators. It is therefore important that none of the
		 * threads change the personList.
		 */
		LT_PersonIdList::iterator first = ltPersonIdList.begin();
		LT_PersonIdList::iterator last = ltPersonIdList.begin() + numPersonsPerThread;
		std::list<std::string>::const_iterator fileNameIt = logFileNames.begin();
		for (int i = 1; i <= numWorkers; i++)
		{
			if (mtConfig.runningPredaySimulation())
			{
				threadGroup.create_thread(boost::bind(&PredayManager::processPersonsForLT_Population, this, first, last, (*fileNameIt)));
				fileNameIt++;
			}
			else if (mtConfig.runningPredayLogsumComputation())
			{
				threadGroup.create_thread(boost::bind(&PredayManager::computeLogsumsForLT_Population, this, first, last));
			}

			first = last;
			if (i + 1 == numWorkers)
			{
				// if the next iteration is the last take all remaining persons
				last = ltPersonIdList.end();
			}
			else
			{
				last = last + numPersonsPerThread;
			}
		}
		threadGroup.join_all();
	}

	// merge log files from each thread into 1 file.
	if (mtConfig.isFileOutputEnabled())
	{
		mergeCSV_Files(logFileNames, logFileNamePrefix);
	}
}

void sim_mob::medium::PredayManager::distributeAndProcessForCalibration(threadedFnPtr fnPtr)
{
	boost::thread_group threadGroup;
	size_t numWorkers = mtConfig.getNumPredayThreads();
	if (numWorkers == 1)
	{ // if single threaded execution was requested
		(this->*fnPtr)(personList.begin(), personList.end(), 0);
	}
	else
	{
		size_t numPersons = personList.size();
		size_t numPersonsPerThread = numPersons / numWorkers;
		PersonList::iterator first = personList.begin();
		PersonList::iterator last = personList.begin() + numPersonsPerThread;
		Print() << "numPersons:" << numPersons << "|numWorkers:" << numWorkers << "|numPersonsPerThread:" << numPersonsPerThread << std::endl;

		/*
		 * Passing different iterators on the same list into the threaded
		 * function. So each thread will iterate mutually exclusive and
		 * exhaustive set of persons from the population.
		 *
		 * Note that each thread will iterate the same personList with different
		 * start and end iterators. It is therefore important that none of the
		 * threads change the personList.
		 */
		for (size_t i = 1; i <= numWorkers; i++)
		{
			threadGroup.create_thread(boost::bind(fnPtr, this, first, last, i - 1));

			first = last;
			if (i + 1 == numWorkers)
			{
				// if the next iteration is the last take all remaining persons
				last = personList.end();
			}
			else
			{
				last = last + numPersonsPerThread;
			}
		}
		threadGroup.join_all();
	}
}

void sim_mob::medium::PredayManager::calibratePreday()
{
	if (!mtConfig.runningPredayCalibration())
	{
		return;
	}
	const PredayCalibrationParams& predayParams = mtConfig.getPredayCalibrationParams();

	/* initialize log file and log header line*/
	logFile = new std::ofstream(mtConfig.getCalibrationOutputFile().c_str());

	{ //begin new scope
		logStream << "iteration#,obj_fn_value,norm_of_gradient";

		/* load variables to calibrate */
		loadCalibrationVariables();
		for (std::vector<CalibrationVariable>::const_iterator calVarIt = calibrationVariablesList.begin(); calVarIt != calibrationVariablesList.end();
				calVarIt++)
		{
			logStream << "," << (*calVarIt).getVariableName();
		}
		constructVariablesInFileMap(calibrationVariablesList);

		/* load observed values for statistics to be computed by simulation */
		CSV_Reader statsReader(predayParams.getObservedStatisticsFile(), true);
		streamVector(statsReader.getHeaderList(), logStream);
		log();
		boost::unordered_map<std::string, std::string> observedValuesMap;
		statsReader.getNextRow(observedValuesMap, false);
		if (observedValuesMap.empty())
		{
			throw std::runtime_error("No data found for observed values of calibration statistics");
		}
		CalibrationStatistics observedStats(observedValuesMap);
		observedStats.getAllStatistics(observedHITS_Stats); // store a local copy for future use

		std::vector<std::string> scalesVector;
		statsReader.getNextRow(scalesVector);
		populateScalesVector(scalesVector);

		logStream << "observed,N/A,N/A";
		streamVector(calibrationVariablesList, logStream);
		streamVector(observedHITS_Stats, logStream);
		log();
	} //scope out unnecessary objects

	/* load weight matrix if required */
	if (mtConfig.runningWSPSA())
	{
		loadWeightMatrix();
	}

	/*initializations*/
	simulatedStatsVector = std::vector<CalibrationStatistics>(mtConfig.getNumPredayThreads());
	unsigned iterationLimit = predayParams.getIterationLimit();

	/* start calibrating */
	RandomSymmetricPlusMinusVector randomVector(calibrationVariablesList.size());
	double initialGradientStepSize = 0, stepSize = 0, normOfGradient = 0;
	objectiveFunctionValues.clear();

	// iteration 0
	std::vector<double> simulatedHITS_Stats;
	Print() << "~~~~~ iteration " << 0 << " ~~~~~\n";
	printParameters(calibrationVariablesList, "iteration");
	double objFn = computeObjectiveFunction(calibrationVariablesList, simulatedHITS_Stats);
	objectiveFunctionValues.push_back(objFn);
	logStream << 0 << "," << objFn << ",N/A";
	streamVector(calibrationVariablesList, logStream);
	streamVector(simulatedHITS_Stats, logStream);
	log();

	for (unsigned k = 1; k <= iterationLimit; k++)
	{
		// iteration k
		Print() << "~~~~~ iteration " << k << " ~~~~~\n";
		simulatedHITS_Stats.clear();

		// 1. compute perturbation step size
		initialGradientStepSize = predayParams.getInitialGradientStepSize() / std::pow((1 + k), predayParams.getAlgorithmCoefficient2());
		Print() << "initialGradientStepSize = " << initialGradientStepSize << std::endl;

		// 2. compute logsums if required
		if ((k % mtConfig.getLogsumComputationFrequency()) == 0)
		{
			distributeAndProcessForCalibration(&PredayManager::computeLogsumsForCalibration);
		}

		// 3. compute gradients using SPSA technique
		std::vector<double> gradientVector;
		if (mtConfig.runningWSPSA())
		{
			computeWeightedGradient(randomVector.get(), initialGradientStepSize, gradientVector);
		}
		else
		{
			computeGradient(randomVector.get(), initialGradientStepSize, gradientVector);
		}
		computeAntiGradient(gradientVector); // checks limits and negates gradient
		printVector("gradient vector", gradientVector);

		// 4. compute norm of the gradient (root of sum of squares of gradients)
		for (std::vector<double>::const_iterator gradIt = gradientVector.begin(); gradIt != gradientVector.end(); gradIt++)
		{
			normOfGradient = normOfGradient + std::pow((*gradIt), 2);
		}
		normOfGradient = std::sqrt(normOfGradient);
		Print() << "norm of gradient vector: " << normOfGradient << std::endl;

		// 5. compute step size
		stepSize = predayParams.getInitialStepSize()
				/ std::pow((predayParams.getInitialStepSize() + k + predayParams.getStabilityConstant()), predayParams.getAlgorithmCoefficient1());
		Print() << "stepSize = " << stepSize << std::endl;

		// 6. update the variables being calibrated and compute objective function
		updateVariablesByAddition(calibrationVariablesList, gradientVector, stepSize);
		printParameters(calibrationVariablesList, "iteration");
		objFn = computeObjectiveFunction(calibrationVariablesList, simulatedHITS_Stats);
		objectiveFunctionValues.push_back(objFn);

		// 7. log current iteration parameters and stats
		logStream << k << "," << objFn << "," << normOfGradient;
		streamVector(calibrationVariablesList, logStream);
		streamVector(simulatedHITS_Stats, logStream);
		log();

		// 8. check termination. At this point there would be atleast k+1 elements in objectiveFunctionValues
		if (std::abs(objectiveFunctionValues[k] - objectiveFunctionValues[k - 1]) <= predayParams.getTolerance())
		{
			break; //converged!
		}
	}

	safe_delete_item(logFile);
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
		} catch (const std::out_of_range& oor)
		{
			throw std::runtime_error("Header mis-match while reading calibration variables csv");
		} catch (boost::bad_lexical_cast const&)
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
	size_t row = 0, col = 0;
	while (!matrixRow.empty())
	{
		std::vector<std::string>::iterator strIt = matrixRow.begin();
		strIt++; //first element is the name of the observation
		for (; strIt != matrixRow.end(); strIt++)
		{
			try
			{
				weightMatrix(row, col) = boost::lexical_cast<double>(*strIt);
			}
			catch (boost::bad_lexical_cast const& badCastEx)
			{
				throw std::runtime_error("Invalid value found in weight matrix csv");
			}
			catch (bad_index const& badIdxEx)
			{
				throw std::runtime_error("weight matrix csv has incorrect number of elements");
			}
			col++;
		}
		row++;
		col = 0;
		matrixRow.clear();
		matrixReader.getNextRow(matrixRow);
	}
}

void sim_mob::medium::PredayManager::processPersonsForCalibration(const PersonList::iterator& firstPersonIt, const PersonList::iterator& oneAfterLastPersonIt,
		size_t threadNum)
{
	CalibrationStatistics& simStats = simulatedStatsVector.at(threadNum);
	bool consoleOutput = mtConfig.isConsoleOutput();

	// time dependent zone-zone travel time data source
	DB_Connection simmobConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().networkDatabase);
	simmobConn.connect();
	if (!simmobConn.isConnected())
	{
		throw std::runtime_error("simmobility db connection failure!");
	}
	TimeDependentTT_SqlDao tcostDao(simmobConn);

	for (PersonList::iterator i = firstPersonIt; i != oneAfterLastPersonIt; i++)
	{
		PredaySystem predaySystem(**i, zoneMap, zoneIdLookup, amCostMap, pmCostMap, opCostMap, tcostDao, unavailableODs);
		predaySystem.planDay();
		predaySystem.updateStatistics(simStats);
		if (consoleOutput)
		{
			predaySystem.printLogs();
		}
	}
}

void sim_mob::medium::PredayManager::computeGradient(const std::vector<short>& randomVector, double initialGradientStepSize,
		std::vector<double>& gradientVector)
{
	size_t numVariables = calibrationVariablesList.size();
	if (randomVector.size() != calibrationVariablesList.size())
	{
		throw std::runtime_error("size mis-match between calibrationVariablesList and randomVector");
	}
	std::vector<double> scaledRandomVector;
	multiplyScalarWithVector(initialGradientStepSize, randomVector, scaledRandomVector);
	std::vector<CalibrationVariable> localCopyCalVarList = calibrationVariablesList;
	updateVariablesByAddition(localCopyCalVarList, scaledRandomVector);
	std::vector<double> simulatedHITS_Stats;
	double objFnPlus = computeObjectiveFunction(localCopyCalVarList, simulatedHITS_Stats);

	localCopyCalVarList = calibrationVariablesList;
	updateVariablesBySubtraction(localCopyCalVarList, scaledRandomVector);
	double objFnMinus = computeObjectiveFunction(localCopyCalVarList, simulatedHITS_Stats);

	gradientVector.clear();
	double varGradient = 0;
	for (size_t i = 0; i < numVariables; i++)
	{
		varGradient = (objFnPlus - objFnMinus) / (2 * scaledRandomVector[i]);
		gradientVector.push_back(varGradient);
	}
}

void sim_mob::medium::PredayManager::computeWeightedGradient(const std::vector<short>& randomVector, double initialGradientStepSize,
		std::vector<double>& gradientVector)
{
	size_t numVariables = calibrationVariablesList.size();
	size_t numStatistics = observedHITS_Stats.size();

	if (randomVector.size() != calibrationVariablesList.size())
	{
		throw std::runtime_error("size mis-match between calibrationVariablesList and randomVector");
	}

	std::vector<double> scaledRandomVector;
	multiplyScalarWithVector(initialGradientStepSize, randomVector, scaledRandomVector);

	std::vector<double> simStatisticsPlus, simStatisticsMinus;

	std::vector<CalibrationVariable> localCopyCalVarList = calibrationVariablesList;
	updateVariablesByAddition(localCopyCalVarList, scaledRandomVector);
	computeObservationsVector(localCopyCalVarList, simStatisticsPlus);
	if (observedHITS_Stats.size() != simStatisticsPlus.size())
	{
		throw std::runtime_error("size mis-match between observed and simulated values");
	}

	localCopyCalVarList = calibrationVariablesList;
	updateVariablesBySubtraction(localCopyCalVarList, scaledRandomVector);
	computeObservationsVector(localCopyCalVarList, simStatisticsMinus);
	if (observedHITS_Stats.size() != simStatisticsMinus.size())
	{
		throw std::runtime_error("size mis-match between observed and simulated values");
	}

	gradientVector.clear();
	double gradient = 0, squaredDifferencePlus = 0, squaredDifferenceMinus = 0, scaledDiff = 0;
	for (size_t j = 0; j < numVariables; j++)
	{
		gradient = 0;
		for (size_t i = 0; i < numStatistics; i++)
		{
			scaledDiff = statsScale[i] * (simStatisticsPlus[i] - observedHITS_Stats[i]);
			squaredDifferencePlus = std::pow(scaledDiff, 2);
			scaledDiff = statsScale[i] * (simStatisticsMinus[i] - observedHITS_Stats[i]);
			squaredDifferenceMinus = std::pow(scaledDiff, 2);
			gradient = gradient + (weightMatrix(i, j) * (squaredDifferencePlus - squaredDifferenceMinus));
		}
		gradient = gradient / (2 * scaledRandomVector[j]);
		gradientVector.push_back(gradient);
	}
}

void sim_mob::medium::PredayManager::processPersonsForLT_Population(const LT_PersonIdList::iterator& firstPersonIdIt,
		const LT_PersonIdList::iterator& oneAfterLastPersonIdIt, const std::string& activityScheduleLog)
{
	bool outputTripchains = mtConfig.isFileOutputEnabled();
	bool consoleOutput = mtConfig.isConsoleOutput();

	// construct population dao specially
	DB_Connection populationConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().populationDatabase);
	populationConn.connect();
	if (!populationConn.isConnected())
	{
		throw std::runtime_error("population db connection failure!");
	}
	PopulationSqlDao populationDao(populationConn);

	// logsum data source
	DB_Connection simmobConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().networkDatabase);
	simmobConn.connect();
	if (!simmobConn.isConnected())
	{
		throw std::runtime_error("simmobility db connection failure!");
	}
	const std::string& logsumTableName = mtConfig.getLogsumTableName();
	SimmobSqlDao logsumSqlDao(simmobConn, logsumTableName);
	TimeDependentTT_SqlDao tcostDao(simmobConn);

	// open log file for this thread
	std::ofstream activityScheduleLogFile(activityScheduleLog.c_str(), std::ios::trunc | std::ios::out);
	std::stringstream activityScheduleStream;

	// loop through all persons within the range and plan their day
	for (LT_PersonIdList::iterator i = firstPersonIdIt; i != oneAfterLastPersonIdIt; i++)
	{
		PersonParams personParams;
		populationDao.getOneById(*i, personParams);
		if (personParams.getPersonId().empty())
		{
			continue;
		} // some persons are not complete in the database
		logsumSqlDao.getLogsumById(*i, personParams);
		PredaySystem predaySystem(personParams, zoneMap, zoneIdLookup, amCostMap, pmCostMap, opCostMap, tcostDao, unavailableODs);
		predaySystem.planDay();

		if (outputTripchains)
		{
			predaySystem.outputActivityScheduleToStream(zoneNodeMap, activityScheduleStream);
			outputToFile(activityScheduleLogFile, activityScheduleStream);
		}
		if (consoleOutput)
		{
			predaySystem.printLogs();
		}
	}
}

void sim_mob::medium::PredayManager::computeLogsumsForCalibration(const PersonList::iterator& firstPersonIt, const PersonList::iterator& oneAfterLastPersonIt,
		size_t threadNum)
{
	bool consoleOutput = mtConfig.isConsoleOutput();

	// time dependent zone-zone travel time data source
	DB_Connection simmobConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().networkDatabase);
	simmobConn.connect();
	if (!simmobConn.isConnected())
	{
		throw std::runtime_error("simmobility db connection failure!");
	}
	TimeDependentTT_SqlDao tcostDao(simmobConn);

	// loop through all persons within the range and plan their day
	for (PersonList::iterator i = firstPersonIt; i != oneAfterLastPersonIt; i++)
	{
		PredaySystem predaySystem(**i, zoneMap, zoneIdLookup, amCostMap, pmCostMap, opCostMap, tcostDao, unavailableODs);
		predaySystem.computeLogsums();
		if (consoleOutput)
		{
			predaySystem.printLogs();
		}
	}
}

void sim_mob::medium::PredayManager::computeLogsumsForLT_Population(const LT_PersonIdList::iterator& firstPersonIdIt,
		const LT_PersonIdList::iterator& oneAfterLastPersonIdIt)
{
	bool consoleOutput = mtConfig.isConsoleOutput();

	// construct population dao specially
	DB_Connection populationConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().populationDatabase);
	populationConn.connect();
	if (!populationConn.isConnected())
	{
		throw std::runtime_error("population db connection failure!");
	}
	PopulationSqlDao populationDao(populationConn);

	// logsum data source
	DB_Connection simmobConn = getDB_Connection(ConfigManager::GetInstance().FullConfig().networkDatabase);
	simmobConn.connect();
	if (!simmobConn.isConnected())
	{
		throw std::runtime_error("simmobility db connection failure!");
	}
	const std::string& logsumTableName = mtConfig.getLogsumTableName();
	SimmobSqlDao logsumSqlDao(simmobConn, logsumTableName);
	TimeDependentTT_SqlDao tcostDao(simmobConn);

	// loop through all persons within the range and plan their day
	for (LT_PersonIdList::iterator i = firstPersonIdIt; i != oneAfterLastPersonIdIt; i++)
	{
		PersonParams personParams;
		populationDao.getOneById(*i, personParams);
		if (personParams.getPersonId().empty())
		{
			continue;
		} // some persons are not complete in the database
		PredaySystem predaySystem(personParams, zoneMap, zoneIdLookup, amCostMap, pmCostMap, opCostMap, tcostDao, unavailableODs);
		predaySystem.computeLogsums();
		logsumSqlDao.insert(personParams);
		if (consoleOutput)
		{
			predaySystem.printLogs();
		}
	}
}

double sim_mob::medium::PredayManager::computeObjectiveFunction(const std::vector<CalibrationVariable>& calVarList, std::vector<double>& simulatedHITS_Stats)
{
	computeObservationsVector(calVarList, simulatedHITS_Stats);
	return computeSumOfDifferenceSquared(observedHITS_Stats, simulatedHITS_Stats);
}

void sim_mob::medium::PredayManager::computeObservationsVector(const std::vector<CalibrationVariable>& calVarList, std::vector<double>& simulatedHITS_Stats)
{
	updateVariablesInLuaFiles(mtConfig.getModelScriptsMap().getPath(), calVarList);
	std::for_each(simulatedStatsVector.begin(), simulatedStatsVector.end(), std::mem_fun_ref(&CalibrationStatistics::reset));
	distributeAndProcessForCalibration(&PredayManager::processPersonsForCalibration);
	simulatedHITS_Stats.clear();
	CalibrationStatistics simulatedStats;
	aggregateStatistics(simulatedStatsVector, simulatedStats);
	simulatedStats.getAllStatistics(simulatedHITS_Stats);
	printVector("simulatedStats", simulatedHITS_Stats);
}

void sim_mob::medium::PredayManager::log()
{
	logStream << std::endl;
	NullableOutputStream(logFile) << logStream.str() << std::flush;
	logStream.str(std::string());
}
