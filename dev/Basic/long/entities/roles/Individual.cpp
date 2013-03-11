/*
 * Individual.cpp
 *
 *  Created on: Mar 5, 2013
 *      Author: gandola
 */

#include "Individual.hpp"

#include "entities/Person.hpp"
#include "entities/UpdateParams.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PartitionManager.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#endif

using namespace sim_mob;
using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;
using std::cout;

namespace sim_mob {

namespace long_term {

const string Individual::ROLE_NAME = "Individual";

Individual::Individual(Agent* parent) :
		Role(parent, Individual::ROLE_NAME), params(gen) {
}

Individual::~Individual() {
}

Role* Individual::clone(Person* parent) const {
	Role* role = new Individual(parent);
	return role;
}

void Individual::frame_init(UpdateParams& p) {
	cout << "Individual::frame_init: " << p.now.frame() << endl;
}

void Individual::frame_tick(UpdateParams& p) {
	cout << "Individual::frame_tick: " << p.now.frame() << endl;
}

void Individual::frame_tick_output(const UpdateParams& p) {
	cout << "Individual::frame_tick_output: " << p.now.frame() << endl;
}

void Individual::frame_tick_output_mpi(timeslice now) {
	throw std::runtime_error(
			"frame_tick_output_mpi not implemented in Individual.");
}

UpdateParams& Individual::make_frame_tick_params(timeslice now) {
	cout << "Individual::make_frame_tick_params: " << now.frame() << endl;
	params.reset(now);
	return params;
}

vector<BufferedBase*> Individual::getSubscriptionParams() {
	cout << "Individual::getSubscriptionParams." << endl;
	vector<BufferedBase*> subcriptionParams; //empty for now.
	return subcriptionParams;
}

void Individual::OnEvent(EventPublisher* sender, EventId id, const EventArgs& args){
        cout << "Individual::OnEvent fired id: " << id << " agentId: " << getParent()->getId() <<endl;
}
#ifndef SIMMOB_DISABLE_MPI
void Individual::pack(PackageUtils& packageUtil) {}
void Individual::unpack(UnPackageUtils& unpackageUtil) {}
void Individual::packProxy(PackageUtils& packageUtil) {}
void Individual::unpackProxy(UnPackageUtils& unpackageUtil) {}
#endif

}
/* namespace long_term */
} /* namespace sim_mob */
