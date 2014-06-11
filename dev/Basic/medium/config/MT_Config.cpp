/*
 * MTConfig.cpp
 *
 *  Created on: 2 Jun, 2014
 *      Author: zhang
 */

#include "MT_Config.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob
{
namespace medium
{
MT_Config::MT_Config() : pedestrianWalkSpeed(0) {}

MT_Config::~MT_Config() {}

MT_Config* MT_Config::instance(nullptr);

MT_Config& MT_Config::GetInstance()
{
	if (!instance)
	{
		instance = new MT_Config();
	}
	return *instance;
}



}
}

