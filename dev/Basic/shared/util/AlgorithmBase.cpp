//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AlgorithmBase.hpp"

#include <limits>

#include "entities/Agent.hpp"
#include "entities/Person.hpp"

using namespace sim_mob;

//This setting is important
int AlgorithmBase::order_by_setting = ORDERING_BY_DRIVING_TIME_TO_INTERSECTION;

/**
 * Inputs: a list of dqueue and an integer
 * Output: Merged dqueue
 * Process:
 * (1) Merge based on vehicle's location
 * (2) The topic Capacity vehicles are merged based on its location
 * (3) After merging top Capacity, the left vehicles are append to the end, without changing their order in lanes.
 * Note:
 * I did not use template in this case, because the persons are ordered based on 2 special attributes: distanceToIntersection and drivingTimeToIntersection
 */
std::deque<sim_mob::Person*> AlgorithmBase::topCMerge(std::vector<std::deque<sim_mob::Person*>* >& all_person_lists, int Capacity) {
	std::deque<sim_mob::Person*> merged_dqueue;
	std::vector<std::deque<sim_mob::Person*>::iterator> all_person_iterator_lists;

	//init location
	int dqueue_size = all_person_lists.size();
	for (std::vector<std::deque<sim_mob::Person*>* >::iterator it = all_person_lists.begin(); it != all_person_lists.end(); ++it) {
		all_person_iterator_lists.push_back((*(*it)).begin());
	}

	//pick the Top C
	for (int i = 0; i < Capacity; i++) {
		int which_queue = -1;
		double min_distance = std::numeric_limits<double>::max();
		sim_mob::Person* which_person = NULL;

		for (int i = 0; i < dqueue_size; i++) {
			//order by location
			if (order_by_setting == ORDERING_BY_DISTANCE_TO_INTERSECTION) {
				if (all_person_iterator_lists[i] != (all_person_lists[i])->end() && (*all_person_iterator_lists[i])->distanceToEndOfSegment < min_distance) {
					which_queue = i;
					min_distance = (*all_person_iterator_lists[i])->distanceToEndOfSegment;
					which_person = (*all_person_iterator_lists[i]);
				}
			}
			//order by time
			else if (order_by_setting == ORDERING_BY_DRIVING_TIME_TO_INTERSECTION){
				if (all_person_iterator_lists[i] != (all_person_lists[i])->end() && (*all_person_iterator_lists[i])->drivingTimeToEndOfLink < min_distance) {
					which_queue = i;
					min_distance = (*all_person_iterator_lists[i])->drivingTimeToEndOfLink;
					which_person = (*all_person_iterator_lists[i]);
				}
			}
		}

		if (which_queue < 0) {
			//no vehicle any more
			return merged_dqueue;
		} else {
			all_person_iterator_lists[which_queue]++;
			merged_dqueue.push_back(which_person);
		}
	}

	//After pick the Top C, there are still some vehicles left in the dqueue
	for (int i = 0; i < dqueue_size; i++) {
		if (all_person_iterator_lists[i] != (all_person_lists[i])->end()) {
			merged_dqueue.insert(merged_dqueue.end(), all_person_iterator_lists[i], (all_person_lists[i])->end());
		}
	}

	return merged_dqueue;
}
