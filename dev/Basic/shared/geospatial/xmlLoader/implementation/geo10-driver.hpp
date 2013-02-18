#pragma once

#include <string>
#include "geospatial/RoadNetwork.hpp"

namespace sim_mob {
namespace xml {

//Sample driver function for our XML parser.
bool InitAndLoadXML(const std::string& fileName, sim_mob::RoadNetwork& resultNetwork, std::map<unsigned int, std::vector<sim_mob::TripChainItem*> >& resultTripChains);

}} //End sim_mob::xml
