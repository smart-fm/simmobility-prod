//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayManager.hpp
 *
 *  Created on: Nov 18, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include "params/PersonParams.hpp"

namespace sim_mob {
namespace medium {
typedef std::vector<PersonParams> PersonList;

class PredayManager {
public:
	/**
	 * Gets person data from the database and stores corresponding PersonParam pointers in personList.
	 */
	void loadPersons();

	/**
	 * Distributes persons to different threads and starts the threads which process the persons
	 */
	void distributeAndProcessPersons(uint16_t numWorkers = 1);

private:
	/**
	 * Threaded function loop.
	 * Loops through all elements in personList and invokes the Preday system of models for each of them.
	 *
	 */
	void processPersons(PersonList& persons);

	PersonList personList;

};
} //end namespace medium
} //end namespace sim_mob



