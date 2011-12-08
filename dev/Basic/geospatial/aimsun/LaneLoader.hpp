/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <vector>
#include "Section.hpp"
#include "Lane.hpp"
#include "Node.hpp"

#include "geospatial/RoadNetwork.hpp"


namespace sim_mob {
namespace aimsun {



/**
 * Static class which helps load the even more notoriously complex Lanes
 */
class LaneLoader {
public:
	static void DecorateLanes(std::map<int, sim_mob::aimsun::Section>& sections, std::vector<sim_mob::aimsun::Lane>& lanes);
	static void GenerateLinkLaneZero(const sim_mob::RoadNetwork& rn, sim_mob::aimsun::Node* start, sim_mob::aimsun::Node* end, std::set<sim_mob::aimsun::Section*> linkSections);
	static void GenerateLinkLanes(const sim_mob::RoadNetwork& rn, std::map<int, sim_mob::aimsun::Node>& nodes, std::map<int, sim_mob::aimsun::Section>& sections);
};




}}
