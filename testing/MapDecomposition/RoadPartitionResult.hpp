/*
 * RoadPartitionResult.hpp
 *
 *  Created on: 03-Feb-2012
 *      Author: xuyan
 */

#pragma once

#include <vector>

#include "RoadBoundaryElement.hpp"
#include "RoadPartitionElement.hpp"

using namespace std;

namespace partitioning {
	class RoadPartitionResult
	{
	public:
		vector<RoadPartitionElement> partitions;
		vector<RoadBoundaryElement> boundarys;
	};
}
