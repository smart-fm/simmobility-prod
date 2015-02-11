/*
 * MTConfig.cpp
 *
 *  Created on: 2 Jun, 2014
 *      Author: zhang
 */

#include "MT_Config.hpp"

#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include "util/LangHelpers.hpp"

namespace sim_mob
{
namespace medium
{

ModelScriptsMap::ModelScriptsMap(const std::string& scriptFilesPath, const std::string& scriptsLang) : path(scriptFilesPath), scriptLanguage(scriptsLang) {}

MongoCollectionsMap::MongoCollectionsMap(const std::string& dbName) : dbName(dbName) {}

PredayCalibrationParams::PredayCalibrationParams() :
	iterationLimit(0), tolerance(0), initialGradientStepSize(0), algorithmCoefficient2(0),
	initialStepSize(0), stabilityConstant(0), algorithmCoefficient1(0)
{}

MT_Config::MT_Config() :
		pedestrianWalkSpeed(0), numPredayThreads(0), configSealed(false), fileOutputEnabled(false),
		consoleOutput(false), predayRunMode(MT_Config::NONE), calibrationMethodology(MT_Config::WSPSA),
		logsumComputationFrequency(0), supplyUpdateInterval(0), activityScheduleLoadInterval(0), busCapacity(0),
		outputPredictions(false)
{}

MT_Config::~MT_Config()
{
	safe_delete_item(instance);
}

MT_Config* MT_Config::instance(nullptr);

MT_Config& MT_Config::getInstance()
{
	if (!instance)
	{
		instance = new MT_Config();
	}
	return *instance;
}

void MT_Config::setPredayRunMode(const std::string runMode)
{
	if(!configSealed)
	{
		if(runMode == "simulation") { predayRunMode = MT_Config::SIMULATION; }
		else if(runMode == "logsum") { predayRunMode = MT_Config::LOGSUM_COMPUTATION; }
		else if(runMode == "calibration") { predayRunMode = MT_Config::CALIBRATION; }
		else if(runMode == "lt_logsum") { predayRunMode = MT_Config::LOGSUM_COMPUTATION_LT; }
		else { throw std::runtime_error("Inadmissible value for preday run_mode"); }
	}
}

unsigned MT_Config::getActivityScheduleLoadInterval() const
{
	return activityScheduleLoadInterval;
}

void MT_Config::setActivityScheduleLoadInterval(unsigned activityScheduleLoadInterval)
{
	if(!configSealed)
	{
		this->activityScheduleLoadInterval = activityScheduleLoadInterval;
	}
}

unsigned MT_Config::getSupplyUpdateInterval() const
{
	return supplyUpdateInterval;
}

void MT_Config::setSupplyUpdateInterval(unsigned supplyUpdateInterval)
{
	if(!configSealed)
	{
		this->supplyUpdateInterval = supplyUpdateInterval;
	}
}

const StoredProcedureMap& MT_Config::getStoredProcedure() const
{
	return storedProcedure;
}

void MT_Config::setStoredProcedureMap(const StoredProcedureMap& storedProcedure)
{
	if(!configSealed)
	{
		this->storedProcedure = storedProcedure;
	}
}

double MT_Config::getPedestrianWalkSpeed() const
{
	return pedestrianWalkSpeed;
}

std::vector<float>& MT_Config::getDwellTimeParams()
{
	return dwellTimeParams;
}

void MT_Config::setPedestrianWalkSpeed(double pedestrianWalkSpeed)
{
	if(!configSealed)
	{
		this->pedestrianWalkSpeed = pedestrianWalkSpeed;
	}
}

unsigned MT_Config::getNumPredayThreads() const
{
	return numPredayThreads;
}

void MT_Config::setNumPredayThreads(unsigned numPredayThreads)
{
	if(!configSealed)
	{
		this->numPredayThreads = numPredayThreads;
	}
}

const ModelScriptsMap& MT_Config::getModelScriptsMap() const
{
	return modelScriptsMap;
}

void MT_Config::setModelScriptsMap(const ModelScriptsMap& modelScriptsMap)
{
	if(!configSealed)
	{
		this->modelScriptsMap = modelScriptsMap;
	}
}

const MongoCollectionsMap& MT_Config::getMongoCollectionsMap() const
{
	return mongoCollectionsMap;
}

void MT_Config::setMongoCollectionsMap(const MongoCollectionsMap& mongoCollectionsMap)
{
	if(!configSealed)
	{
		this->mongoCollectionsMap = mongoCollectionsMap;
	}
}

void MT_Config::sealConfig()
{
	configSealed = true;
}

const PredayCalibrationParams& MT_Config::getPredayCalibrationParams() const
{
	if(runningWSPSA()) { return wspsaCalibrationParams; }
	return spsaCalibrationParams;
}

const PredayCalibrationParams& MT_Config::getSPSA_CalibrationParams() const
{
	return spsaCalibrationParams;
}

void MT_Config::setSPSA_CalibrationParams(const PredayCalibrationParams& predayCalibrationParams)
{
	if(!configSealed)
	{
		this->spsaCalibrationParams = predayCalibrationParams;
	}
}

const PredayCalibrationParams& MT_Config::getWSPSA_CalibrationParams() const
{
	return wspsaCalibrationParams;
}

bool MT_Config::isFileOutputEnabled() const
{
	return fileOutputEnabled;
}

void MT_Config::setFileOutputEnabled(bool outputTripchains)
{
	if(!configSealed)
	{
		this->fileOutputEnabled = outputTripchains;
	}
}

bool MT_Config::isOutputPredictions() const
{
	return outputPredictions;
}

void MT_Config::setOutputPredictions(bool outputPredictions)
{
	if(!configSealed)
	{
		this->outputPredictions = outputPredictions;
	}
}

bool MT_Config::isConsoleOutput() const
{
	return consoleOutput;
}

void MT_Config::setConsoleOutput(bool consoleOutput)
{
	if(!configSealed)
	{
		this->consoleOutput = consoleOutput;
	}
}

bool MT_Config::runningPredaySimulation() const
{
	return (predayRunMode == MT_Config::SIMULATION);
}

bool MT_Config::runningPredayCalibration() const
{
	return (predayRunMode == MT_Config::CALIBRATION);
}

bool MT_Config::runningPredayLogsumComputation() const
{
	return (predayRunMode == MT_Config::LOGSUM_COMPUTATION);
}

bool MT_Config::runningPredayLogsumComputationForLT() const
{
	return (predayRunMode == MT_Config::LOGSUM_COMPUTATION_LT);
}

bool MT_Config::runningSPSA() const
{
	return (calibrationMethodology == MT_Config::SPSA);
}

bool MT_Config::runningWSPSA() const
{
	return (calibrationMethodology == MT_Config::WSPSA);
}

const std::string& MT_Config::getCalibrationOutputFile() const
{
	return calibrationOutputFile;
}

void MT_Config::setCalibrationOutputFile(const std::string& calibrationOutputFile)
{
	if(!configSealed)
	{
		this->calibrationOutputFile = calibrationOutputFile;
	}
}

void MT_Config::setCalibrationMethodology(const std::string calibrationMethod)
{
	if(!configSealed)
	{
		std::string method = boost::to_upper_copy(calibrationMethod);
		if(method == "WSPSA") { calibrationMethodology = MT_Config::WSPSA; }
		else if(method == "SPSA") { calibrationMethodology = MT_Config::SPSA; }
	}
}

void MT_Config::setWSPSA_CalibrationParams(const PredayCalibrationParams& predayCalibrationParams)
{
	if(!configSealed)
	{
		if(runningWSPSA() && predayCalibrationParams.getWeightMatrixFile().empty())
		{
			throw std::runtime_error("weight matrix is not provided for WSPSA");
		}
		this->wspsaCalibrationParams = predayCalibrationParams;
	}
}

unsigned MT_Config::getLogsumComputationFrequency() const
{
	return logsumComputationFrequency;
}

void MT_Config::setLogsumComputationFrequency(unsigned logsumComputationFrequency)
{
	if(!configSealed)
	{
		this->logsumComputationFrequency = logsumComputationFrequency;
	}
}

const std::string& MT_Config::getFilenameOfJourneyTimeStats() const {
	return filenameOfJourneyTimeStats;
}

const std::string& MT_Config::getFilenameOfWaitingTimeStats() const {
	return filenameOfWaitingTimeStats;
}

void MT_Config::setFilenameOfJourneyTimeStats(const std::string& str) {
	if(!configSealed)
	{
		filenameOfJourneyTimeStats = str;
	}
}

void MT_Config::setFilenameOfWaitingTimeStats(const std::string& str) {
	if(!configSealed)
	{
		filenameOfWaitingTimeStats = str;
	}
}

const std::string& MT_Config::getFilenameOfWaitingAmountStats() const {
	return filenameOfWaitingAmountStats;
}

void MT_Config::setFilenameOfWaitingAmountStats(const std::string& str) {
	if(!configSealed)
	{
		filenameOfWaitingAmountStats = str;
	}
}

const unsigned int MT_Config::getBusCapacity() const
{
	return busCapacity;
}

void MT_Config::setBusCapacity(const unsigned int busCapacity)
{
	if(!configSealed)
	{
		this->busCapacity = busCapacity;
	}
}

db::BackendType MT_Config::getPopulationSource() const
{
	return populationSource;
}

void MT_Config::setPopulationSource(const std::string& src)
{
	if(!configSealed)
	{
		std::string dataSourceStr = boost::to_upper_copy(src);
		if(dataSourceStr == "PGSQL") { populationSource = db::POSTGRES; }
		else { populationSource = db::MONGO_DB; } //default setting
	}
}

}
}
