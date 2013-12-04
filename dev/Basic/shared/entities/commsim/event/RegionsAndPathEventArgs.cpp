//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RegionsAndPathEventArgs.hpp"
#include "entities/Agent.hpp"

using namespace sim_mob;

sim_mob::RegionsAndPathEventArgs::RegionsAndPathEventArgs(const sim_mob::Agent * agent_, const std::vector<sim_mob::RoadRunnerRegion>& all_regions, const std::vector<sim_mob::RoadRunnerRegion>& region_path) :
	agent(agent_), all_regions(all_regions), region_path(region_path)
{
}

sim_mob::RegionsAndPathEventArgs::~RegionsAndPathEventArgs()
{
}

Json::Value sim_mob::RegionsAndPathEventArgs::toJSON() const
{
	//TODO: Replace with Regions and Paths, actual serialized format.
	return JsonParser::makeRegionAndPathMessage(all_regions, region_path);
}


