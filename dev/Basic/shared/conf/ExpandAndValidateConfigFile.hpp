//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/ConfigParams.hpp"

#include <boost/noncopyable.hpp>
#include <set>

namespace sim_mob {

class ConfigParams;
class Entity;


/**
 * Class which takes a ConfigParams object with the RawConfigParams already loaded and expands them
 *   into the full set of ConfigParams. Also validates certain items (like tick length, etc.)
 * Typically used like a verb:
 *     ConfigParams& cfg = ConfigParams::getInstanceRW();
 *     ExpandAndValidateConfigFile print(cfg);
 *
 * \todo
 * For now, this class has a lot of "action" items mixed in to the expand-and-validate. For example,
 * agents are generated and the network is printed. These should eventually be reshuffled; this class should
 * *only* set and validate parameters in ConfigParams.
 *
 * \note
 * This class is actually USED by the old config format (simpleconf). Don't delete it if you are cleaning
 * up the remains of the new config format (which doesn't work at the moment). ~Seth
 */
class ExpandAndValidateConfigFile : private boost::noncopyable {
public:
	///Perform further semantic processing, and confirm that parameters are set correctly.
    ExpandAndValidateConfigFile(ConfigParams& result);

private:
    ///Does all the work.
    void processConfig();


};

}
