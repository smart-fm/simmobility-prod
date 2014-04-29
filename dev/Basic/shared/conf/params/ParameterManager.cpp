/*
 * ParameterManager.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Max
 */

#include "ParameterManager.hpp"

#include <stdexcept>


namespace sim_mob {

ParameterManager * ParameterManager::instance = NULL;
ParameterManager *ParameterManager::Instance()
{
	if(!instance)
	{
		instance = new ParameterManager();
	}
	return instance;
}
ParameterManager::ParameterManager() {
	// TODO Auto-generated constructor stub
	ParseParamFile ppfile("data/driver_behavior_model/driver_param.xml",this);
}

ParameterManager::~ParameterManager() {
	// TODO Auto-generated destructor stub
}
void ParameterManager::setParam(const std::string& modelName, const std::string& key, const ParamData& v)
{
	ParameterPoolConIterator it = parameterPool.find(modelName);
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
		ParameterNameValueMapConIterator itt = nvMap.find(key);
		if(itt!=nvMap.end())
		{
			std::string s= "param already exit: "+key;
			throw std::runtime_error(s);
		}
		nvMap.insert(std::make_pair(key,v));
		parameterPool.insert(std::make_pair(modelName,nvMap));
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
