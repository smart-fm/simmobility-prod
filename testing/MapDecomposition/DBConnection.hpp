/*
 * loader.hpp
 *
 *  Created on: 03-Feb-2012
 *      Author: xuyan
 */

#pragma once

#include "soci.h"
#include "soci-postgresql.h"

#include "RoadSection.hpp"
#include "RoadNode.hpp"
#include "RoadPartitionResult.hpp"
#include "Configuration.hpp"

#include <iostream>

using namespace std;

namespace partitioning {
 class DBConnection
 {
 public:
	 explicit DBConnection();

	 vector<RoadNode> getAllRoadNode();
	 vector<RoadSection> getAllRoadSection();
	 bool savePartitionResult(RoadPartitionResult& result);

 private:
	 soci::session sql_;
 };
}
