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
void ParameterManager::setParam(const std::string& key, const ParamData& v)
{
	ParameterPoolConIterator it = parameterPool.find(key);
	if(it!=parameterPool.end())
	{
		std::string s= "param already exit: "+key;
		throw std::runtime_error(s);
	}
	parameterPool.insert(std::make_pair(key,v));
}
void ParameterManager::setParam(const std::string& key, const std::string& s)
{
	ParamData v(s);
	parameterPool.insert(std::make_pair(key,v));
}
void ParameterManager::setParam(const std::string& key, double d)
{
	ParamData v(d);
	parameterPool.insert(std::make_pair(key,v));
}
void ParameterManager::setParam(const std::string& key, int i)
{
	ParamData v(i);
	parameterPool.insert(std::make_pair(key,v));
}
void ParameterManager::setParam(const std::string& key, bool b)
{
	ParamData v(b);
	parameterPool.insert(std::make_pair(key,v));
}
bool ParameterManager::hasParam(const std::string& key) const
{
	ParameterPoolConIterator it = parameterPool.find(key);
	if(it != parameterPool.end())
	{
		return true;
	}
	return false;
}
bool ParameterManager::getParam(const std::string& key, double& d) const
{
	ParamData v;
	if (!getParam(key, v))
	{
		return false;
	}

	d = v.toDouble();

	return true;
}
bool ParameterManager::getParam(const std::string& key, ParamData& v) const
{
	ParameterPoolConIterator it = parameterPool.find(key);
	if(it != parameterPool.end())
	{
		return true;
	}
	v = it->second;
	return false;
}
}//namespace sim_mob
