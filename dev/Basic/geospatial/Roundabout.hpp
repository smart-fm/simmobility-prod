/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include "Node.hpp"
#include "Lane.hpp"

namespace sim_mob
{

/**
 * A node where multiple Links join. By themselves, Nodes provide all of the information
 * required to diagram which Lanes lead to other lanes at a given roundabout. The
 * Roundabout class, however, adds a much-needed layer of context by specifying exactly
 * how the roundabout actually looks and behaves.
 */
class Roundabout : public sim_mob::Node {
public:

protected:
	std::vector<bool> separator;

	std::vector<float> percentChunks;

	std::vector<float> offsets;

	std::vector<float> entranceAngle;

	std::vector<int> addDominantLane;

	float roundaboutDominantIslands;

	int roundaboutNumberOfLanes;

	std::vector<RoadItem*> obstacles;
};





}
