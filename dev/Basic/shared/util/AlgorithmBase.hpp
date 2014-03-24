//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * The class implements some algorithms which are not belonging to any particular class.
 * Thus, the algorithm can be shared by any class anywhere.
 */

#pragma once

#include <vector>
#include <deque>

namespace sim_mob {

class Person;

enum CONFLUX_VEHICLE_ORDER
{
   ORDERING_BY_DISTANCE_TO_INTERSECTION,
   ORDERING_BY_DRIVING_TIME_TO_INTERSECTION
};

class AlgorithmBase {
public:

	/*
	 * TopCMergeDqueue
	 */
	static std::deque<sim_mob::Person*> topCMerge(std::vector< std::deque<sim_mob::Person*>* >& all_person_lists, int Capacity);

	static int order_by_setting;
};

}
