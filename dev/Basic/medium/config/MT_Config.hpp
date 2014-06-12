/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 *
 * File:   MT_Config.hpp
 * Author: zhang huai peng
 *
 * Created on 2 Jun, 2014
 */

#pragma once
#include <string>
#include <map>
#include <vector>

#include "util/ProtectedCopyable.hpp"
namespace sim_mob
{
namespace medium
{

class ModelScriptsMap
{
public:
	ModelScriptsMap(const std::string& scriptFilesPath = "", const std::string& scriptsLang = "");

	const std::string& getPath() const
	{
		return path;
	}

	const std::string& getScriptLanguage() const
	{
		return scriptLanguage;
	}

	std::string getScriptFileName(std::string key) const
	{
		//at() is used intentionally so that an out_of_range exception is triggered when invalid key is passed
		return scriptFileNameMap.at(key);
	}

	void addScriptFileName(const std::string& key, const std::string& value)
	{
		this->scriptFileNameMap[key] = value;
	}

private:
	std::string path;
	std::string scriptLanguage;
	std::map<std::string, std::string> scriptFileNameMap; //key=>value
};

class MongoCollectionsMap
{
public:
	MongoCollectionsMap(const std::string& dbName = "");

	std::string getCollectionName(std::string key) const
	{
		//at() is used intentionally so that an out_of_range exception is triggered when invalid key is passed
		return collectionNameMap.at(key);
	}

	const std::map<std::string, std::string>& getCollectionsMap() const
	{
		return collectionNameMap;
	}

	void addCollectionName(const std::string& key, const std::string& value)
	{
		this->collectionNameMap[key] = value;
	}

	const std::string& getDbName() const
	{
		return dbName;
	}

private:
	std::string dbName;
	std::map<std::string, std::string> collectionNameMap; //key=>value
};

class PredayCalibrationParams
{
public:
	PredayCalibrationParams();

	const std::string& getCalibrationVariablesFile() const
	{
		return calibrationVariablesFile;
	}

	void setCalibrationVariablesFile(const std::string& calibrationVariablesFile)
	{
		this->calibrationVariablesFile = calibrationVariablesFile;
	}

	unsigned getIterationLimit() const
	{
		return iterationLimit;
	}

	void setIterationLimit(unsigned iterationLimit)
	{
		this->iterationLimit = iterationLimit;
	}

	double getPertubationStepSizeConst() const
	{
		return pertubationStepSizeConst;
	}

	void setPertubationStepSizeConst(double pertubationStepSizeConst)
	{
		this->pertubationStepSizeConst = pertubationStepSizeConst;
	}

	double getPertubationStepSizeExponent() const
	{
		return pertubationStepSizeExponent;
	}

	void setPertubationStepSizeExponent(double pertubationStepSizeExponent)
	{
		this->pertubationStepSizeExponent = pertubationStepSizeExponent;
	}

	double getStepSizeConst() const
	{
		return stepSizeConst;
	}

	void setStepSizeConst(double stepSizeConst)
	{
		this->stepSizeConst = stepSizeConst;
	}

	double getStepSizeExponent() const
	{
		return stepSizeExponent;
	}

	void setStepSizeExponent(double stepSizeExponent)
	{
		this->stepSizeExponent = stepSizeExponent;
	}

	double getTolerance() const
	{
		return tolerance;
	}

	void setTolerance(double tolerance)
	{
		this->tolerance = tolerance;
	}

private:
	/**path and file name of csv containing variables to calibrate*/
	std::string calibrationVariablesFile;
	unsigned iterationLimit;
	double tolerance;
	double pertubationStepSizeConst;
	double pertubationStepSizeExponent;
	double stepSizeConst;
	double stepSizeExponent;
};

class MT_Config : private sim_mob::ProtectedCopyable
{
public:
	MT_Config();
	virtual ~MT_Config();

	static MT_Config& GetInstance();

	double getPedestrianWalkSpeed() const
	{
		return pedestrianWalkSpeed;
	}

	std::vector<int>& getDwellTimeParams()
	{
		return dwellTimeParams;
	}

	void setPedestrianWalkSpeed(double pedestrianWalkSpeed)
	{
		if(!configSealed)
		{
			this->pedestrianWalkSpeed = pedestrianWalkSpeed;
		}
	}

	unsigned getNumPredayThreads() const
	{
		return numPredayThreads;
	}

	void setNumPredayThreads(unsigned numPredayThreads)
	{
		if(!configSealed)
		{
			this->numPredayThreads = numPredayThreads;
		}
	}

	const ModelScriptsMap& getModelScriptsMap() const
	{
		return modelScriptsMap;
	}

	void setModelScriptsMap(const ModelScriptsMap& modelScriptsMap)
	{
		if(!configSealed)
		{
			this->modelScriptsMap = modelScriptsMap;
		}
	}

	const MongoCollectionsMap& getMongoCollectionsMap() const
	{
		return mongoCollectionsMap;
	}

	void setMongoCollectionsMap(const MongoCollectionsMap& mongoCollectionsMap)
	{
		if(!configSealed)
		{
			this->mongoCollectionsMap = mongoCollectionsMap;
		}
	}

	bool isPredayCalibrationMode() const
	{
		return predayCalibrationMode;
	}

	void setPredayCalibrationMode(bool predayCalibrationMode)
	{
		if(!configSealed)
		{
			this->predayCalibrationMode = predayCalibrationMode;
		}
	}

	void sealConfig()
	{
		configSealed = true;
	}

	const PredayCalibrationParams& getPredayCalibrationParams() const
	{
		return predayCalibrationParams;
	}

	void setPredayCalibrationParams(const PredayCalibrationParams& predayCalibrationParams)
	{
		if(!configSealed)
		{
			this->predayCalibrationParams = predayCalibrationParams;
		}
	}

private:
	static MT_Config* instance;

	/**protection for changes after config is loaded*/
	bool configSealed;
	/**store parameters for dwelling time calculation*/
	std::vector<int> dwellTimeParams;
	/**store parameters for pedestrian walking speed*/
	double pedestrianWalkSpeed;
	/**num of threads to run for preday*/
	unsigned numPredayThreads;
	/**container for lua scripts*/
	ModelScriptsMap modelScriptsMap;
	/**container for mongo collections*/
	MongoCollectionsMap mongoCollectionsMap;
	/**flag to indicate whether calibration is enabled*/
	bool predayCalibrationMode;
	/**Preday calibration parameters*/
	PredayCalibrationParams predayCalibrationParams;
};
}
}

