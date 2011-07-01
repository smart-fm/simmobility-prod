#pragma once

#include <vector>

#include "Node.hpp"
#include "Lane.hpp"

namespace sim_mob
{

/**
 * Part of a Link with consistent rules.
 */
class RoadSegment : public Pavement {
public:


private:
	std::vector<Lane> lanes;


};





}
