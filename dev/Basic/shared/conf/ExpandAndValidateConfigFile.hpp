/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <boost/noncopyable.hpp>

namespace sim_mob {

class ConfigParams;


/**
 * Class which takes a ConfigParams object with the RawConfigParams already loaded and expands them
 *   into the full set of ConfigParams. Also validates certain items (like tick length, etc.)
 * Typically used like a verb:
 *     ConfigParams& cfg = ConfigParams::getInstanceRW();
 *     ExpandAndValidateConfigFile print(cfg);
 *
 * \note
 * This class is actually USED by the old config format (simpleconf). Don't delete it if you are cleaning
 * up the remains of the new config format (which doesn't work at the moment). ~Seth
 */
class ExpandAndValidateConfigFile : private boost::noncopyable {
public:
	///Perform further semantic processing, and confirm that parameters are set correctly.
	ExpandAndValidateConfigFile(ConfigParams& result);

protected:
	///Does all the work.
	void ProcessConfig();

private:
	//These functions are called by ProcessConfig()
	void CheckGranularities();

private:
	//The config file we are currently loading
	ConfigParams& cfg;
};

}
