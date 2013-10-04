//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <limits>

#include "entities/models/LaneChangeModel.hpp"
#include "geospatial/Lane.hpp"

using std::numeric_limits;
using namespace sim_mob;



double sim_mob::SimpleLaneChangeModel::executeLaneChanging(DriverUpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir, LANE_CHANGE_MODE mode)
{
  return 1; //TODO

}
