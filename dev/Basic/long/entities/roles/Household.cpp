/*
 * Household.cpp
 *
 *  Created on: Mar 5, 2013
 *      Author: gandola
 */

#include "Household.hpp"

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

const string Household::ROLE_NAME = "Household";

Household::Household(Agent* parent) :
		Role(parent, Household::ROLE_NAME), params(gen) {
}

Household::~Household() {
}

Role* Household::clone(Person* parent) const {
	Role* role = new Household(parent);
	return role;
}

void Household::frame_init(UpdateParams& p) {
	cout << "Household::frame_init: " << p.now.frame() << endl;
}

void Household::frame_tick(UpdateParams& p) {
	cout << "Household::frame_tick: " << p.now.frame() << endl;
}

void Household::frame_tick_output(const UpdateParams& p) {
	cout << "Household::frame_tick_output: " << p.now.frame() << endl;
}

void Household::frame_tick_output_mpi(timeslice now) {
	throw std::runtime_error(
			"frame_tick_output_mpi not implemented in Household.");
}

UpdateParams& Household::make_frame_tick_params(timeslice now) {
	cout << "Household::make_frame_tick_params: " << now.frame() << endl;
	params.reset(now);
	return params;
}

vector<BufferedBase*> Household::getSubscriptionParams() {
	cout << "Household::getSubscriptionParams." << endl;
	vector<BufferedBase*> subcriptionParams; //empty for now.
	return subcriptionParams;
}

void Household::OnEvent(EventPublisher* sender, EventId id, const EventArgs& args){
  cout << "Household::OnEvent fired id: " << id << " agentId: " << getParent()->getId() <<endl;  
}

#ifndef SIMMOB_DISABLE_MPI
void Household::pack(PackageUtils& packageUtil) {}
void Household::unpack(UnPackageUtils& unpackageUtil) {}
void Household::packProxy(PackageUtils& packageUtil) {}
void Household::unpackProxy(UnPackageUtils& unpackageUtil) {}
#endif

}
/* namespace long_term */
} /* namespace sim_mob */
