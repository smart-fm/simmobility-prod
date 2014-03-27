//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RegionsAndPathEventArgs.hpp"

#include "entities/commsim/serialization/CommsimSerializer.hpp"


using namespace sim_mob;

sim_mob::RegionsAndPathEventArgs::RegionsAndPathEventArgs(const sim_mob::Agent* agent, const std::vector<sim_mob::RoadRunnerRegion>& allRegions, const std::vector<sim_mob::RoadRunnerRegion>& regionPath) :
	agent(agent), allRegions(allRegions), regionPath(regionPath)
{
}

sim_mob::RegionsAndPathEventArgs::~RegionsAndPathEventArgs()
{
}

std::string sim_mob::RegionsAndPathEventArgs::serialize() const
{
	//TODO: Replace with Regions and Paths, actual serialized format.
	return CommsimSerializer::makeRegionAndPath(allRegions, regionPath);
}

/*Json::Value sim_mob::RegionsAndPathEventArgs::toJSON() const
{
	//TODO: Replace with Regions and Paths, actual serialized format.
	return JsonParser::makeRegionAndPathMessage(all_regions, region_path);
}*/
