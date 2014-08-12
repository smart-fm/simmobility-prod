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

	double getAlgorithmCoefficient1() const
	{
		return algorithmCoefficient1;
	}

	void setAlgorithmCoefficient1(double algorithmCoefficient1)
	{
		this->algorithmCoefficient1 = algorithmCoefficient1;
	}

	double getAlgorithmCoefficient2() const
	{
		return algorithmCoefficient2;
	}

	void setAlgorithmCoefficient2(double algorithmCoefficient2)
	{
		this->algorithmCoefficient2 = algorithmCoefficient2;
	}

	const std::string& getCalibrationVariablesFile() const
	{
		return calibrationVariablesFile;
	}

	void setCalibrationVariablesFile(const std::string& calibrationVariablesFile)
	{
		this->calibrationVariablesFile = calibrationVariablesFile;
	}

	double getInitialGradientStepSize() const
	{
		return initialGradientStepSize;
	}

	void setInitialGradientStepSize(double initialGradientStepSize)
	{
		this->initialGradientStepSize = initialGradientStepSize;
	}

	double getInitialStepSize() const
	{
		return initialStepSize;
	}

	void setInitialStepSize(double initialStepSize)
	{
		this->initialStepSize = initialStepSize;
	}

	unsigned getIterationLimit() const
	{
		return iterationLimit;
	}

	void setIterationLimit(unsigned iterationLimit)
	{
		this->iterationLimit = iterationLimit;
	}

	double getStabilityConstant() const
	{
		return stabilityConstant;
	}

	void setStabilityConstant(double stabilityConstant)
	{
		this->stabilityConstant = stabilityConstant;
	}

	double getTolerance() const
	{
		return tolerance;
	}

	void setTolerance(double tolerance)
	{
		this->tolerance = tolerance;
	}

	const std::string& getObservedStatisticsFile() const
	{
		return observedStatisticsFile;
	}

	void setObservedStatisticsFile(const std::string& observedStatisticsFile)
	{
		this->observedStatisticsFile = observedStatisticsFile;
	}

	const std::string& getWeightMatrixFile() const
	{
		return weightMatrixFile;
	}

	void setWeightMatrixFile(const std::string& weightMatrixFile)
	{
		this->weightMatrixFile = weightMatrixFile;
	}

private:
	/**path and file name of csv containing variables to calibrate*/
	std::string calibrationVariablesFile;
	unsigned iterationLimit;
	double tolerance;
	double initialGradientStepSize;
	double algorithmCoefficient2;
	double initialStepSize;
	double stabilityConstant;
	double algorithmCoefficient1;
	std::string observedStatisticsFile;
	std::string weightMatrixFile;
};

class MT_Config : private sim_mob::ProtectedCopyable
{
public:
	virtual ~MT_Config();

	static MT_Config& getInstance();

	double getPedestrianWalkSpeed() const;
	std::vector<int>& getDwellTimeParams();
	void setPedestrianWalkSpeed(double pedestrianWalkSpeed);
	unsigned getNumPredayThreads() const;
	void setNumPredayThreads(unsigned numPredayThreads);
	const ModelScriptsMap& getModelScriptsMap() const;
	void setModelScriptsMap(const ModelScriptsMap& modelScriptsMap);
	const MongoCollectionsMap& getMongoCollectionsMap() const;
	void setMongoCollectionsMap(const MongoCollectionsMap& mongoCollectionsMap);

	/** the object of this class gets sealed when this function is called. No more changes will be allowed via the setters */
	void sealConfig();

	const PredayCalibrationParams& getPredayCalibrationParams() const;
	const PredayCalibrationParams& getSPSA_CalibrationParams() const;
	void setSPSA_CalibrationParams(const PredayCalibrationParams& predayCalibrationParams);
	const PredayCalibrationParams& getWSPSA_CalibrationParams() const;
	void setWSPSA_CalibrationParams(const PredayCalibrationParams& predayCalibrationParams);
	bool isOutputTripchains() const;
	void setOutputTripchains(bool outputTripchains);
	bool isOutputPredictions() const;
	void setOutputPredictions(bool outputPredictions);
	bool isConsoleOutput() const;
	void setConsoleOutput(bool consoleOutput);
	bool runningPredaySimulation() const;
	bool runningPredayCalibration() const;
	bool runningPredayLogsumComputation() const;
	void setPredayRunMode(const std::string runMode);
	bool runningSPSA() const;
	bool runningWSPSA() const;
	void setCalibrationMethodology(const std::string calibrationMethod);
	const std::string& getCalibrationOutputFile() const;
	void setCalibrationOutputFile(const std::string& calibrationOutputFile);
	unsigned getLogsumComputationFrequency() const;
	void setLogsumComputationFrequency(unsigned logsumComputationFrequency);

private:
	MT_Config();
	static MT_Config* instance;

	/**protection for changes after config is loaded*/
	bool configSealed;
	/**store parameters for dwelling time calculation*/
	std::vector<int> dwellTimeParams;
	/**store parameters for pedestrian walking speed*/
	double pedestrianWalkSpeed;

	/**control variable for running preday simulation/logsum computation*/
	enum PredayRunMode { NONE, SIMULATION, CALIBRATION, LOGSUM_COMPUTATION };
	PredayRunMode predayRunMode;

	/**num of threads to run for preday*/
	unsigned numPredayThreads;
	/**flag to indicate whether tripchains need to be output to postgresql db*/
	bool outputTripchains;
	/**flag to indicate whether tours and stops need to be output in mongodb*/
	bool outputPredictions;
	/**flag to indicate whether console output is required*/
	bool consoleOutput;
	/**container for lua scripts*/
	ModelScriptsMap modelScriptsMap;
	/**container for mongo collections*/
	MongoCollectionsMap mongoCollectionsMap;

	/**Preday calibration parameters*/
	enum CalibrationMethodology { SPSA, WSPSA };
	CalibrationMethodology calibrationMethodology;
	PredayCalibrationParams spsaCalibrationParams;
	PredayCalibrationParams wspsaCalibrationParams;
	std::string calibrationOutputFile;
	unsigned logsumComputationFrequency;
};
}
}

