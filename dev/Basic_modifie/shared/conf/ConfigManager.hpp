//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "ConfigParams.hpp"

namespace sim_mob
{

///Forward declarations
class RawConfigParams;
class CMakeConfigParams;
class PathSetConf;

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
class ConfigManager
{
public:
	/**
	 * Constructor
	 */
	ConfigManager();

	/**
	 * Destructor
	 */
	~ConfigManager();

	/**
	 * Retrieves a const reference to the sigleton instance of ConfigManager
	 *
	 * @return const reference to the singleton instance of ConfigManager
	 */
	static const ConfigManager& GetInstance();

	/**
	 * Retrieves a reference to the singleton instance of ConfigManager
	 *
	 * @return reference to the singleton instance of ConfigManager
	 */
	static ConfigManager& GetInstanceRW();

	/**
	 * Deletes the singleton instance ConfigManager
	 */
	static void DeleteConfigMgrInstance();

	/**
	 * Retrieves a reference to ConfigParams object
	 *
	 * @return reference to ConfigParams object
	 */
	ConfigParams& FullConfig();

	/**
	 * Retrieves a const reference to ConfigParams object
	 *
	 * @return const reference to ConfigParams object
	 */
	const ConfigParams& FullConfig() const;

	/**
	 * Retrieves a const reference to pathset configuration structure
	 *
	 * @return const reference to pathset configuration
	 */
	const PathSetConf& PathSetConfig() const;

	/**
	 * Retrieves a reference to pathset configuration structure
	 *
	 * @return reference to pathset configuration
	 */
	PathSetConf& PathSetConfig();

	/**
	 * Retrieves a reference to RawConfigParams object
	 *
	 * @return reference to RawConfigParams object
	 */
	RawConfigParams& XmlConfig();

	/**
	 * Retrieves a const reference to RawConfigParams object
	 *
	 * @return const reference to RawConfigParams object
	 */
	const RawConfigParams& XmlConfig() const;

	/**
	 * Retrieves a reference to CMakeConfigParams object.
	 *
	 * @return reference to CMakeConfigParams object
	 */
	CMakeConfigParams& CMakeConfig();

	/**
	 * Retrieves a const reference to CMakeConfigParams object.
	 *
	 * @return const reference to CMakeConfigParams object
	 */
	const CMakeConfigParams& CMakeConfig() const;

	/**
	 * Reset this instance of the static ConfigParams instance.
	 * WARNING: This should *only* be used by the interactive loop of Sim Mobility.
	 */
	void reset();

private:
	/**
	 * Helper, single point of creation for ConfigParams
	 *
	 * @return reference to ConfigParams object
	 */
	ConfigParams& get_config() const;

	/**
	 * Helper, single point of creation for ConfigParams and can read write;
	 *
	 * @return reference to ConfigParams object
	 */
	ConfigParams& get_config_rw();

	/**Singleton instance*/
	static ConfigManager* instance;

	/**ConfigParams object, We create on retrieval, mostly for convenience. Hence, mutable.*/
	mutable ConfigParams* config;
};
}
