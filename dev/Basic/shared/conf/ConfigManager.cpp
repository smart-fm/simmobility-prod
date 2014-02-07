//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ConfigManager.hpp"

#include "conf/CMakeConfigParams.hpp"
#include "conf/RawConfigParams.hpp"
#include "conf/ConfigParams.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob;


ConfigManager* sim_mob::ConfigManager::instance(nullptr);

sim_mob::ConfigManager::ConfigManager() : config(nullptr)
{}

sim_mob::ConfigManager::~ConfigManager()
{
	safe_delete_item(config);
}


const ConfigManager& sim_mob::ConfigManager::GetInstance()
{
	return GetInstanceRW();
}

ConfigManager& sim_mob::ConfigManager::GetInstanceRW()
{
	if (!instance) {
		instance = new ConfigManager();
	}
	return *instance;
}

ConfigParams& sim_mob::ConfigManager::get_config() const
{
	if (!config) {
		config = new ConfigParams();
	}
	return *config;
}
ConfigParams& sim_mob::ConfigManager::get_config_rw()
{
	if (!config) {
		config = new ConfigParams();
	}
	return *config;
}

ConfigParams& sim_mob::ConfigManager::FullConfig()
{
	return get_config_rw();
}

const ConfigParams& sim_mob::ConfigManager::FullConfig() const
{
	return get_config();
}

RawConfigParams& sim_mob::ConfigManager::XmlConfig()
{
	return get_config();
}

const RawConfigParams& sim_mob::ConfigManager::XmlConfig() const
{
	return get_config();
}

CMakeConfigParams& sim_mob::ConfigManager::CMakeConfig()
{
	return get_config();
}

const CMakeConfigParams& sim_mob::ConfigManager::CMakeConfig() const
{
	return get_config();
}

void sim_mob::ConfigManager::reset()
{
	//TODO: This *should* work fine, since it will re-generate the instance on re-creation.
	safe_delete_item(instance);
}

