/*
 * RoadSection.hpp
 *
 *  Created on: 03-Feb-2012
 *      Author: xuyan
 */

#pragma once

#include "RoadNode.hpp"

namespace partitioning {
class RoadSection {
public:
	int section_id;

	int from_node_id;
	int to_node_id;

	RoadNode* from_node;
	RoadNode* to_node;

	int lane_size;
	double section_length;
};
}
