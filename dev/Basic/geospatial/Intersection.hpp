/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include "Node.hpp"

namespace sim_mob
{

/**
 * A node where two Links join. By themselves, Nodes provide all of the information
 * required to diagram which Lanes lead to other lanes at a given intersection. The
 * Intersection class, however, adds a much-needed layer of context by specifying exactly
 * how the intersection actually looks and behaves.
 */
class Intersection : public sim_mob::Node {
public:

protected:
	std::vector<float> percentChunks;
	std::vector<float> roadAngles;


};





}
