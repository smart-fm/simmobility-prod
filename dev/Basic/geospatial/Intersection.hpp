/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include "MultiNode.hpp"
#include "Lane.hpp"

namespace sim_mob
{


namespace aimsun
{
//Forward declarations
class Loader;
}



/**
 * A node where multiple Links join. By themselves, Nodes provide all of the information
 * required to diagram which Lanes lead to other lanes at a given intersection. The
 * Intersection class, however, adds a much-needed layer of context by specifying exactly
 * how the intersection actually looks and behaves.
 */
class Intersection : public sim_mob::MultiNode {
public:

protected:
	std::vector<float> separator;

	std::vector<float> percentChunks;

	std::vector<float> offsets;

	std::vector<sim_mob::Lane> additionalDominantLanes;

	std::vector<sim_mob::Lane> additionalSubdominantLanes;

	std::vector<bool> dominantIslands;

	std::vector<RoadItem*> obstacles;

friend class aimsun::Loader;

};





}
