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

	std::string path;
	std::string scriptLanguage;
	std::map<std::string, std::string> scriptFileName; //key=>value
};

class MongoCollectionsMap
{
public:
	MongoCollectionsMap(const std::string& dbName = "");

	std::string dbName;
	std::map<std::string, std::string> collectionName; //key=>value
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
		this->pedestrianWalkSpeed = pedestrianWalkSpeed;
	}

	unsigned getNumPredayThreads() const
	{
		return numPredayThreads;
	}

	void setNumPredayThreads(unsigned numPredayThreads)
	{
		this->numPredayThreads = numPredayThreads;
	}

	const ModelScriptsMap& getModelScriptsMap() const
	{
		return modelScriptsMap;
	}

	const std::string& getFilenameOfJourneyStats() const
	{
		return filenameOfJourneyStats;
	}

	const std::string& getFilenameOfWaitingStats() const
	{
		return filenameOfWaitingStats;
	}

	void setFilenameOfJourneyStats(const std::string& str)
	{
		filenameOfJourneyStats = str;
	}

	void setFilenameOfWaitingStats(const std::string& str)
	{
		filenameOfWaitingStats = str;
	}

	void setModelScriptsMap(const ModelScriptsMap& modelScriptsMap)
	{
		this->modelScriptsMap = modelScriptsMap;
	}

	const MongoCollectionsMap& getMongoCollectionsMap() const
	{
		return mongoCollectionsMap;
	}

	void setMongoCollectionsMap(const MongoCollectionsMap& mongoCollectionsMap)
	{
		this->mongoCollectionsMap = mongoCollectionsMap;
	}

	const unsigned int getBusCapcacity() const
	{
		return busCapcacity;
	}

	void setBusCapcacity(const unsigned int busCapcacity)
	{
		this->busCapcacity = busCapcacity;
	}

private:
	static MT_Config* instance;
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
	/**the filename of storing journey statistics */
	std::string filenameOfJourneyStats;
	/**the filename of storing waiting time statistics*/
	std::string filenameOfWaitingStats;
	/**default capacity for bus*/
	unsigned int busCapcacity;
};
}
}

