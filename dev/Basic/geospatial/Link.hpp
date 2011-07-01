#pragma once

#include <vector>

#include "Node.hpp"
#include "RoadSegment.hpp"

namespace sim_mob
{

/**
 * A road or sidewalk. Generalized movement rules apply for agents inside a link,
 * which is itself composed of segments.
 */
class Link {
public:


private:
	Node startPos;
	Node endPos;

	//NOTE: Might need to consider sidewalks separately.
	std::vector<RoadSegment> segments;


};





}
