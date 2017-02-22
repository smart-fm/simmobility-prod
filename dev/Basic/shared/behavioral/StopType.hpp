//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob
{
/**
 * Fundamental stop (activity) types supported by preday.
 *
 * \author Harish Loganathan
 */
typedef int StopType;

const StopType NULL_STOP = -1;

const int WORK_ACTIVITY_TYPE = 1;
const int EDUCATION_ACTIVITY_TYPE = 2;
const int OTHER_ACTIVITY_TYPE = 3;

const int PT_TRAVEL_MODE = 1;
const int PRIVATE_BUS_MODE = 2;
const int PVT_CAR_MODE = 3;
const int SHARING_MODE = 4;
const int PVT_BIKE_MODE = 5;
const int WALK_MODE = 6;
const int TAXI_MODE = 7;

}
