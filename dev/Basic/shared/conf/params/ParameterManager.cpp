/*
 * ParameterManager.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Max
 */

#include "ParameterManager.hpp"
#include <string>
#include <stdexcept>

namespace sim_mob {

ParameterManager::ParameterManager() {
	// TODO Auto-generated constructor stub
	ParseParamFile ppfile("data/driver_behavior_model/driver_param.xml",this);
}

ParameterManager::~ParameterManager() {
	// TODO Auto-generated destructor stub
}
void ParameterManager::setParam(const std::string& key, const XmlRpc::XmlRpcValue& v)
{
	ParameterPoolConIterator it = parameterPool.find(key);
	if(it!=parameterPool.end())
	{
		std::string s= "param already exit: "+key;
		throw std::runtime_error(s);
	}
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
	XmlRpc::XmlRpcValue v;
	if (!getParam(key, v))
	{
		return false;
	}

	if (v.getType() == XmlRpc::XmlRpcValue::TypeInt)
	{
		d = (int)v;
	}
	else if (v.getType() != XmlRpc::XmlRpcValue::TypeDouble)
	{
		return false;
	}
	else
	{
		d = v;
	}

	return true;
}
bool ParameterManager::getParam(const std::string& key, XmlRpc::XmlRpcValue& v) const
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
