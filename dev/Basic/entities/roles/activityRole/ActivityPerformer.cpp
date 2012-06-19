/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Pedestrian.cpp
 *
 *  Created on: 2011-6-20
 *      Author: Linbo
 */

#include "ActivityPerformer.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "geospatial/Node.hpp"
#include "util/OutputUtil.hpp"

using std::vector;
using namespace sim_mob;

sim_mob::ActivityPerformer::ActivityPerformer(Agent* parent) :
	Role(parent) {
	//Check non-null parent. Perhaps references may be of use here?
	if (!parent) {
		std::cout << "Role constructed with no parent Agent." << std::endl;
		throw 1;
	}
}
