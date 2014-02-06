//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once


namespace sim_mob {

//Forward declarations
class ConfigParams;
class RawConfigParams;
class CMakeConfigParams;



/**
 * A lightweight wrapper for ConfigParams that manages a singleton instance.
 * Features: minimal header includes and polymorphic return types (for minimizing dependencies in cpp files).
 *
 * \note
 * If you retrieve a configuration from this manager, you will *still* need to #include the file which contains
 * that class's description. For example, if you retrieve XmlConfig(), you will still need to include "conf/RawConfigParams.hpp"
 *
 * \author Seth N. Hetu
 */
class ConfigManager {
public:
	ConfigManager();
	~ConfigManager();

	//Retrieve this ConfigManager as a const or non-const reference.
	static const ConfigManager& GetInstance();
	static ConfigManager& GetInstanceRW();

	//Retrieve the ConfigParams singleton as a ConfigParams object.
	ConfigParams& FullConfig();
	const ConfigParams& FullConfig() const;

	//Retrieve the ConfigParams singleton as a RawConfigParams object.
	RawConfigParams& XmlConfig();
	const RawConfigParams& XmlConfig() const;

	//Retrieve the ConfigParams singleton as a CMakeConfigParams object.
	CMakeConfigParams& CMakeConfig();
	const CMakeConfigParams& CMakeConfig() const;

	///Reset this instance of the static ConfigParams instance.
	///WARNING: This should *only* be used by the interactive loop of Sim Mobility.
	void reset();

private:
	ConfigParams& get_config() const; ///<Helper, single point of creatino for ConfigParams;
	ConfigParams& get_config_rw(); ///<Helper, single point of creatino for ConfigParams and can read write;

	static ConfigManager* instance;
	mutable ConfigParams* config;  //We create on retrieval, mostly for convenience. Hence, mutable.
};


}
