/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file Pedestrian.cpp
 *
 *  \author Harish
 */

#include "ActivityPerformer.hpp"
#include "entities/Person.hpp"
#include "geospatial/Node.hpp"
#include "util/OutputUtil.hpp"

using std::vector;
using namespace sim_mob;

sim_mob::ActivityPerformer::ActivityPerformer(Agent* parent) :
	Role(parent), params(parent->getGenerator()) {
	//Check non-null parent. Perhaps references may be of use here?
	if (!parent) {
		std::cout << "Role constructed with no parent Agent." << std::endl;
		throw 1;
	}
}

sim_mob::ActivityPerformer::~ActivityPerformer(){}

sim_mob::ActivityPerformerUpdateParams::~ActivityPerformerUpdateParams(){};

void sim_mob::ActivityPerformer::frame_init(UpdateParams& p) {
}

void sim_mob::ActivityPerformer::frame_tick(UpdateParams& p) {
}

void sim_mob::ActivityPerformer::frame_tick_output(const UpdateParams& p) {
}

void sim_mob::ActivityPerformer::frame_tick_output_mpi(frame_t frameNumber) {
}

UpdateParams& sim_mob::ActivityPerformer::make_frame_tick_params(
		frame_t frameNumber, unsigned int currTimeMS) {
}

std::vector<sim_mob::BufferedBase*> sim_mob::ActivityPerformer::getSubscriptionParams() {
}

