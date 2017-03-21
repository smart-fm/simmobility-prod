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

MongoCollectionsMap::MongoCollectionsMap(const std::string& dbName) : dbName(dbName) {}

PredayCalibrationParams::PredayCalibrationParams() :
	iterationLimit(0), tolerance(0), initialGradientStepSize(0), algorithmCoefficient2(0),
	initialStepSize(0), stabilityConstant(0), algorithmCoefficient1(0)
{}

MT_Config::MT_Config() :
       regionRestrictionEnabled(false), midTermRunMode(MT_Config::MT_NONE), pedestrianWalkSpeed(0), numPredayThreads(0),
			configSealed(false), fileOutputEnabled(false), consoleOutput(false), predayRunMode(MT_Config::PREDAY_NONE),
			calibrationMethodology(MT_Config::WSPSA), logsumComputationFrequency(0), supplyUpdateInterval(0),
			activityScheduleLoadInterval(0), busCapacity(0), populationSource(db::MONGO_DB), granPersonTicks(0),threadsNumInPersonLoader(0)
{
}

MT_Config::~MT_Config()
{
    clear_delete_vector(confluxes);
    clear_delete_vector(segmentStatsWithBusStops);
    clear_delete_map(multinode_confluxes);

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
		if(runMode == "simulation") { predayRunMode = MT_Config::PREDAY_SIMULATION; }
		else if(runMode == "logsum") { predayRunMode = MT_Config::PREDAY_LOGSUM_COMPUTATION; }
		else if(runMode == "calibration") { predayRunMode = MT_Config::PREDAY_CALIBRATION; }
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

void MT_Config::setSPSA_CalibrationParams(const PredayCalibrationParams& spsaCalibrationParams)
{
	if(!configSealed)
	{
        this->spsaCalibrationParams = spsaCalibrationParams;
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
	return (predayRunMode == MT_Config::PREDAY_SIMULATION);
}

bool MT_Config::runningPredayCalibration() const
{
	return (predayRunMode == MT_Config::PREDAY_CALIBRATION);
}

bool MT_Config::runningPredayLogsumComputation() const
{
	return (predayRunMode == MT_Config::PREDAY_LOGSUM_COMPUTATION);
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

void MT_Config::setWSPSA_CalibrationParams(const PredayCalibrationParams& wspsaCalibrationParams)
{
	if(!configSealed)
	{
        if(runningWSPSA() && wspsaCalibrationParams.getWeightMatrixFile().empty())
		{
			throw std::runtime_error("weight matrix is not provided for WSPSA");
		}
        this->wspsaCalibrationParams = wspsaCalibrationParams;
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

const std::string& MT_Config::getLogsumTableName() const
{
	return logsumTableName;
}

void MT_Config::setLogsumTableName(const std::string& logsumTableName)
{
	if(!configSealed)
	{
		this->logsumTableName = logsumTableName;
	}
}
const unsigned int MT_Config::getThreadsNumInPersonLoader() const
{
	return threadsNumInPersonLoader;
}

void MT_Config::setThreadsNumInPersonLoader(unsigned int number)
{
	if(!configSealed)
	{
		threadsNumInPersonLoader = number;
	}
}

bool MT_Config::RunningMidSupply() const {
    return (midTermRunMode == MT_Config::MT_SUPPLY);
}

bool MT_Config::RunningMidDemand() const {
    return (midTermRunMode == MT_Config::MT_PREDAY);
}

void MT_Config::setMidTermRunMode(const std::string& runMode)
{
    if(runMode.empty()) { return; }
    if(runMode == "supply" || runMode == "withinday")
    {
        midTermRunMode = MT_Config::MT_SUPPLY;
    }
    else if (runMode == "preday")
    {
        midTermRunMode = MT_Config::MT_PREDAY;
    }
    else
    {
        throw std::runtime_error("inadmissible value for mid_term_run_mode. Must be either 'supply' or 'preday'");
    }
}

bool MT_Config::isRegionRestrictionEnabled() const{
    return regionRestrictionEnabled;
}

std::vector<IncidentParams>& MT_Config::getIncidents(){
    return incidents;
}

std::map<const Node*, Conflux*>& MT_Config::getConfluxNodes()
{
    return multinode_confluxes;
}

const std::map<const Node*, Conflux*>& MT_Config::getConfluxNodes() const
{
    return multinode_confluxes;
}

Conflux* MT_Config::getConfluxForNode(const Node* multinode) const
{
    std::map<const Node*, Conflux*>::const_iterator cfxIt = multinode_confluxes.find(multinode);
    if(cfxIt == multinode_confluxes.end()) { return nullptr; }
    return cfxIt->second;
}

std::set<Conflux*>& MT_Config::getConfluxes()
{
    return confluxes;
}

const std::set<Conflux*>& MT_Config::getConfluxes() const
{
    return confluxes;
}

std::set<SegmentStats*>& MT_Config::getSegmentStatsWithBusStops()
{
    return segmentStatsWithBusStops;
}

std::set<SegmentStats*>& MT_Config::getSegmentStatsWithTaxiStands()
{
	return segmentStatsWithTaxiStands;
}

unsigned int MT_Config::personTimeStepInMilliSeconds() const
{
    return workers.person.granularityMs;
}

const WorkerParams& MT_Config::getWorkerParams() const
{
	return workers;
}

unsigned int& sim_mob::medium::MT_Config::personWorkGroupSize()
{
	return workers.person.count;
}

unsigned int sim_mob::medium::MT_Config::personWorkGroupSize() const
{
	return workers.person.count;
}

SpeedDensityParams sim_mob::medium::MT_Config::getSpeedDensityParam(int linkCategory) const
{
	if(linkCategory < 1 || linkCategory > 7)
	{
		throw std::runtime_error("invalid link category passed to fetch speed density parameters");
	}
	return speedDensityParams[linkCategory-1];
}

void sim_mob::medium::MT_Config::setSpeedDensityParam(int linkCategory, SpeedDensityParams sdParams)
{
	if(!configSealed)
	{
		if(linkCategory < 1 || linkCategory > 7)
		{
			throw std::runtime_error("invalid link category passed to set speed density parameters");
		}
		speedDensityParams[linkCategory-1] = sdParams;
	}
}

}
}
