
#include "ParameterManager.hpp"

#include <iostream>
#include <stdexcept>

#include "config/ST_Config.hpp"

using namespace sim_mob;

std::map<InstanceType, ParameterManager *> ParameterManager::instances;

ParameterManager *ParameterManager::Instance(bool isAMOD_InstanceRequested)
{
	ParameterManager *instance = NULL;
	InstanceType instanceType;
	map<InstanceType, ParameterManager *>::iterator itInstances;

	//Set the required instance type
	if (isAMOD_InstanceRequested)
	{
		instanceType = ParameterMgrInstance_AMOD;
	}
	else
	{
		instanceType = ParameterMgrInstance_Normal;
	}

	//Check if the required instance is already in our map
	itInstances = instances.find(instanceType);

	//If the required instance does not exist, we need to add it to out map
	if (itInstances == instances.end())
	{
		//Create new instance
		instance = new ParameterManager(isAMOD_InstanceRequested);

		//Add it to the map
		instances.insert(make_pair(instanceType, instance));
	}
	else
	{
		instance = itInstances->second;
	}

	return instance;
}

ParameterManager::ParameterManager(bool isAMOD_InstanceRequeseted)
{
	//Get the specified driver behaviour file from the configuration
	const ST_Config& configParams = ST_Config::getInstance();
	std::string filePathProperty;

	if (isAMOD_InstanceRequeseted)
	{
		filePathProperty = "amod_behaviour_file";
	}
	else
	{
		filePathProperty = "driver_behaviour_file";
	}

	std::map<std::string, std::string>::const_iterator itProperty = configParams.genericProps.find(filePathProperty);

	if (itProperty != configParams.genericProps.end())
	{
		ParseParamFile ppfile(itProperty->second, this);
	}
	else
	{
		stringstream msg;
		msg << __func__ << ": Parameter file - " << filePathProperty << " - not specified in the configuration file!";
		throw runtime_error(msg.str());
	}
}

ParameterManager::~ParameterManager()
{
	//Delete the parameter manager objects
	map<InstanceType, ParameterManager *>::iterator itInstances = instances.begin();

	while (itInstances != instances.end())
	{
		delete itInstances->second;
		++itInstances;
	}
}

void ParameterManager::setParam(const std::string& modelName, const std::string& key, const ParamData& v)
{
	ParameterPoolIterator it = parameterPool.find(modelName);
	if (it == parameterPool.end())
	{
		// new model
		ParameterNameValueMap nvMap;
		nvMap.insert(std::make_pair(key, v));
		parameterPool.insert(std::make_pair(modelName, nvMap));
	}
	else
	{
		ParameterNameValueMap nvMap = it->second;
		ParameterNameValueMapIterator itt = nvMap.find(key);
		
		if (itt != nvMap.end())
		{
			std::stringstream s;
			s << __func__ << ": Parameter " << key << " has already been added!";
			throw std::runtime_error(s.str());
		}

		nvMap.insert(std::make_pair(key, v));

		parameterPool[modelName] = nvMap;
	}

}

void ParameterManager::setParam(const std::string& modelName, const std::string& key, const std::string& s)
{
	ParamData v(s);
	setParam(modelName, key, v);
}

void ParameterManager::setParam(const std::string& modelName, const std::string& key, double d)
{
	ParamData v(d);
	setParam(modelName, key, v);
}

void ParameterManager::setParam(const std::string& modelName, const std::string& key, int i)
{
	ParamData v(i);
	setParam(modelName, key, v);
}

void ParameterManager::setParam(const std::string& modelName, const std::string& key, bool b)
{
	ParamData v(b);
	setParam(modelName, key, v);
}

bool ParameterManager::hasParam(const std::string& modelName, const std::string& key) const
{
	ParameterPoolConIterator it = parameterPool.find(modelName);
	
	if (it == parameterPool.end())
	{
		return false;
	}
	
	ParameterNameValueMap nvMap = it->second;
	ParameterNameValueMapConIterator itt = nvMap.find(key);
	
	if (itt == nvMap.end())
	{
		return false;
	}
	return true;
}

bool ParameterManager::getParam(const std::string& modelName, const std::string& key, double& d) const
{
	ParamData v;
	
	if (!getParam(modelName, key, v))
	{
		return false;
	}

	d = v.toDouble();

	return true;
}

bool ParameterManager::getParam(const std::string& modelName, const std::string& key, std::string& s) const
{
	ParamData v;
	
	if (!getParam(modelName, key, v))
	{
		return false;
	}

	s = v.toString();

	return true;
}

bool ParameterManager::getParam(const std::string& modelName, const std::string& key, ParamData& v) const
{
	ParameterPoolConIterator it = parameterPool.find(modelName);
	
	if (it == parameterPool.end())
	{
		return false;
	}
	
	ParameterNameValueMap nvMap = it->second;
	ParameterNameValueMapConIterator itt = nvMap.find(key);
	
	if (itt == nvMap.end())
	{
		return false;
	}
	
	v = itt->second;
	return true;
}
