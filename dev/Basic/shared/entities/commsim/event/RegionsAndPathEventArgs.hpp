//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
/*
#include <set>
#include <vector>

#include "event/EventListener.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"
#include "entities/commsim/event/JsonSerializableEventArgs.hpp"
#include "geospatial/RoadRunnerRegion.hpp"

namespace sim_mob {
class Agent;
class RegionsAndPathEventArgs : public sim_mob::comm::JsonSerializableEventArgs {
public:*/
	/**
	 * The "all_regions" set is the set of all RoadRunnerRegions, but ONLY if this set has changed since the last update.
	 * The "region_path" vector is a list of RoadRunnerRegions that the Agent must negotiate, but ONLY if this has changed.
	 *    Note that RoadRunner will update the "region_path" itself, so you only need to send a new path if a re-routing
	 *    decision has occurred (e.g., you do NOT need to send a new path if the Agent moves through a region as expected).
	 */
	/*RegionsAndPathEventArgs(const sim_mob::Agent* ag, const std::vector<sim_mob::RoadRunnerRegion>& all_regions, const std::vector<sim_mob::RoadRunnerRegion>& region_path);

	virtual ~RegionsAndPathEventArgs();

	virtual Json::Value toJSON()const;

private:
	const sim_mob::Agent *agent;
	std::vector<sim_mob::RoadRunnerRegion> all_regions;
	std::vector<sim_mob::RoadRunnerRegion> region_path;

};

}*/ /* namespace sim_mob */
