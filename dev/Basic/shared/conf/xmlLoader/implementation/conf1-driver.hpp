#pragma once

#include <string>
#include "conf/Config.hpp"

namespace sim_mob {
namespace xml {

//Sample driver function for our XML parser.
bool InitAndLoadConfigXML(const std::string& fileName, sim_mob::Config& resultConfig);

}} //End sim_mob::xml
