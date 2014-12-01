/*
 * ParameterManager.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Max
 */

#include "ParameterManager.hpp"

#include <stdexcept>
#include <iostream>


namespace sim_mob {

std::map<InstanceType, ParameterManager *> ParameterManager::instances;

ParameterManager *ParameterManager::Instance(bool isAMOD_InstanceRequested)
{
	ParameterManager *instance = NULL;
	InstanceType instanceType;
	map<InstanceType, ParameterManager *>::iterator itInstances;

	//Set the required instance type
	if(isAMOD_InstanceRequested)
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
	if(itInstances == instances.end())
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
	if(isAMOD_InstanceRequeseted)
	{
		ParseParamFile ppfile("data/driver_behavior_model/amod_driver_param.xml",this);
	}
	else
	{
		ParseParamFile ppfile("data/driver_behavior_model/driver_param.xml",this);
	}
}

ParameterManager::~ParameterManager()
{
	//Delete the parameter manager objects

	map<InstanceType, ParameterManager *>::iterator itInstances = instances.begin();

	while(itInstances != instances.end())
	{
		delete itInstances->second;
		itInstances++;
	}
}

void ParameterManager::setParam(const std::string& modelName, const std::string& key, const ParamData& v)
{
	std::cout<<modelName<<" "<<key<<std::endl;
	ParameterPoolIterator it = parameterPool.find(modelName);
	if(it==parameterPool.end())
	{
		// new model
		ParameterNameValueMap nvMap;
		nvMap.insert(std::make_pair(key,v));
		parameterPool.insert(std::make_pair(modelName,nvMap));
	}
	else
	{
		ParameterNameValueMap nvMap = it->second;
//		std::cout<<"nvMap size bf: "<<nvMap.size()<<std::endl;
		ParameterNameValueMapIterator itt = nvMap.find(key);
		if(itt!=nvMap.end())
		{
			std::string s= "param already exit: "+key;
			throw std::runtime_error(s);
		}
		//
//		for(itt=nvMap.begin();itt!=nvMap.end();++itt)
//		{
//			std::cout<<"key: "<<itt->first<<std::endl;
//		}
		nvMap.insert(std::make_pair(key,v));

//		std::cout<<"nvMap size: "<<nvMap.size()<<std::endl;
		parameterPool[modelName]=nvMap;
//		//
//		ParameterPoolIterator ittt = parameterPool.find(modelName);
//		ParameterNameValueMap nvMapt = ittt->second;
//		std::cout<<"nvMapt size: "<<nvMapt.size()<<std::endl;
	}

}
void ParameterManager::setParam(const std::string& modelName, const std::string& key, const std::string& s)
{
	ParamData v(s);
	setParam(modelName,key,v);
}
void ParameterManager::setParam(const std::string& modelName, const std::string& key, double d)
{
	ParamData v(d);
	setParam(modelName,key,v);
}
void ParameterManager::setParam(const std::string& modelName, const std::string& key, int i)
{
	ParamData v(i);
	setParam(modelName,key,v);
}
void ParameterManager::setParam(const std::string& modelName, const std::string& key, bool b)
{
	ParamData v(b);
	setParam(modelName,key,v);
}
bool ParameterManager::hasParam(const std::string& modelName, const std::string& key) const
{
	ParameterPoolConIterator it = parameterPool.find(modelName);
	if(it == parameterPool.end())
	{
		return false;
	}
	ParameterNameValueMap nvMap = it->second;
	ParameterNameValueMapConIterator itt = nvMap.find(key);
	if(itt==nvMap.end())
	{
		return false;
	}
	return true;
}
bool ParameterManager::getParam(const std::string& modelName, const std::string& key, double& d) const
{
	ParamData v;
	if (!getParam(modelName,key, v))
	{
		return false;
	}

	d = v.toDouble();

	return true;
}
bool ParameterManager::getParam(const std::string& modelName, const std::string& key, std::string& s) const
{
	ParamData v;
	if (!getParam(modelName,key, v))
	{
		return false;
	}

	s = v.toString();

	return true;
}
bool ParameterManager::getParam(const std::string& modelName, const std::string& key, ParamData& v) const
{
	ParameterPoolConIterator it = parameterPool.find(modelName);
	if(it == parameterPool.end())
	{
		return false;
	}
	ParameterNameValueMap nvMap = it->second;
	ParameterNameValueMapConIterator itt = nvMap.find(key);
	if(itt == nvMap.end())
	{
		return false;
	}
	v = itt->second;
	return true;
}
}//namespace sim_mob
