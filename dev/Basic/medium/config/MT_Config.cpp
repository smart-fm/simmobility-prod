/*
 * MTConfig.cpp
 *
 *  Created on: 2 Jun, 2014
 *      Author: zhang
 */

#include "MT_Config.hpp"

#include <stdexcept>
#include "util/LangHelpers.hpp"

namespace sim_mob
{
namespace medium
{

ModelScriptsMap::ModelScriptsMap(const std::string& scriptFilesPath, const std::string& scriptsLang) : path(scriptFilesPath), scriptLanguage(scriptsLang) {}

MongoCollectionsMap::MongoCollectionsMap(const std::string& dbName) : dbName(dbName) {}

PredayCalibrationParams::PredayCalibrationParams() :
	iterationLimit(0), tolerance(0), pertubationStepSizeConst(0), pertubationStepSizeExponent(0),
	stepSizeConst(0), stepSizeExponent(0)
{}

MT_Config::MT_Config() :
		pedestrianWalkSpeed(0), numPredayThreads(0), configSealed(false), outputTripchains(false),
consoleOutput(false), predayRunMode(MT_Config::NONE)
{}

MT_Config::~MT_Config() {}

MT_Config* MT_Config::instance(nullptr);

MT_Config& MT_Config::GetInstance()
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
		else { throw std::runtime_error("Inadmissible value for preday run_mode"); }
	}
}


}
}

