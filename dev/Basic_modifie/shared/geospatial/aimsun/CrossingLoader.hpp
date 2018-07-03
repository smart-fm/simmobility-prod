//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include "Node.hpp"
#include "Crossing.hpp"

namespace sim_mob {

class RoadNetwork;

namespace aimsun {



/**
 * Static class which helps load the notoriously complex Crossings
 * \author Seth N. Hetu
 * \author Matthew Bremer Bruchon
 */
class CrossingLoader {
public:
	static void DecorateCrossings(std::map<int, sim_mob::aimsun::Node>& nodes, std::vector<sim_mob::aimsun::Crossing>& crossing);
	static void GenerateACrossing(sim_mob::RoadNetwork& resNW, sim_mob::aimsun::Node& origin, sim_mob::aimsun::Node& dest, std::vector<int>& laneIDs);


};




}}
