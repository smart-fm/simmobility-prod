/*
 * METISHelper.hpp
 *
 *  Created on: 03-Feb-2012
 *      Author: xuyan
 */

#pragma once

#include "metis.h"
#include <vector>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include "RoadNode.hpp"
#include "RoadSection.hpp"

using namespace std;

namespace partitioning {
class METISHelper
 {
 public:
	vector<partitioning::RoadNode>* all_nodes;
	vector<partitioning::RoadSection>* all_sections;

 public:
	void getMETISParameters(idx_t* node_array, idx_t* link_array, idx_t* node_weight, int node_size, int link_size);

// private:
//	int getNodeIndexByID(int node_id);
 };
}
