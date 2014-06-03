/*
 * MTConfig.cpp
 *
 *  Created on: 2 Jun, 2014
 *      Author: zhang
 */

#include "MT_Config.hpp"

namespace sim_mob {
namespace medium {
MT_Config::MT_Config() {
	// TODO Auto-generated constructor stub

}

MT_Config::~MT_Config() {
	// TODO Auto-generated destructor stub
}

MT_Config* MT_Config::instance(nullptr);

MT_Config& MT_Config::GetInstance()
{
	if (!instance) {
		instance = new MT_Config();
	}
	return *instance;
}

void MT_Config::processElement(xercesc::DOMElement* node,  const std::string& name)
{

}
}
}

