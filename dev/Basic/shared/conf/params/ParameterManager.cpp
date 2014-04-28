/*
 * ParameterManager.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Max
 */

#include "ParameterManager.hpp"

namespace sim_mob {

ParameterManager::ParameterManager() {
	// TODO Auto-generated constructor stub

}

ParameterManager::~ParameterManager() {
	// TODO Auto-generated destructor stub
}

bool ParameterManager::getParam(const std::string& key, double& d) const
{
	XmlRpc::XmlRpcValue v;
	if (!getXmlRpcValue(key, v))
	  {
	    return false;
	  }

	  if (v.getType() == XmlRpc::XmlRpcValue::TypeDouble)
	  {
	    double d = v;

	    if (fmod(d, 1.0) < 0.5)
	    {
	      d = floor(d);
	    }
	    else
	    {
	      d = ceil(d);
	    }

	    i = d;
	  }
	  else if (v.getType() != XmlRpc::XmlRpcValue::TypeInt)
	  {
	    return false;
	  }
	  else
	  {
	    i = v;
	  }

	  return true;
}

}//namespace sim_mob
