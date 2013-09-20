//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <map>

#include "geospatial/RoadNetwork.hpp"

namespace sim_mob {

//Forward declarations
class TripChainItem;

namespace xml {

//Sample driver function for our XML parser.
bool InitAndLoadXML(const std::string& fileName, sim_mob::RoadNetwork& resultNetwork, std::map<std::string, std::vector<sim_mob::TripChainItem*> >& resultTripChains);

//Similar function, but only loads trip chains
bool InitAndLoadTripChainsFromXML(const std::string& fileName, const std::string& rootNode, std::map<std::string, std::vector<sim_mob::TripChainItem*> >& resultTripChains);

}} //End sim_mob::xml
