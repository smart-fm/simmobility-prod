//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <string>
#include <set>
#include <map>
#include <iostream>
#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "soci/soci.h"
#include "soci/postgresql/soci-postgresql.h"
#include "path/Common.hpp"
#include "path/PathSetParam.hpp"

namespace sim_mob
{

//Forward declarations
class Link;
class Node;

namespace aimsun
{

/**
 * Class for loading AimSun data and converting it to a format compatible with out network.
 */
class Loader 
{
public:
	/**Get all CBD nodes*/
	static void getCBD_Nodes(std::map<unsigned int, const sim_mob::Node*>& nodes);

	/*get all CBD's links*/
	static void getCBD_Links(std::set<const sim_mob::Link*> & zoneLinks);
};

}

}
