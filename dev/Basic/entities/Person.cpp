/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Person.hpp"

//For debugging
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "util/DebugFlags.hpp"

using std::vector;
using namespace sim_mob;

sim_mob::Person::Person(int id) :
	Agent(id), currRole(nullptr), currTripChain(nullptr) {

}

sim_mob::Person::~Person() {
	safe_delete(currRole);
}

bool sim_mob::Person::update(frame_t frameNumber) {
	//Update this agent's role
	if (currRole) {
		currRole->update(frameNumber);
	}

	if (Debug::WorkGroupSemantics && isToBeRemoved()) {
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout << "Person requested removal: " << (dynamic_cast<Driver*> (currRole) ? "Driver"
				: dynamic_cast<Pedestrian*> (currRole) ? "Pedestrian" : "Other") << "\n";
	}

	//Return true unless we are scheduled for removal.
	//NOTE: Make sure you set this flag AFTER performing your final output.
	return !isToBeRemoved();
}

void sim_mob::Person::output(frame_t frameNumber) {
	if (currRole) {
		currRole->output(frameNumber);
	}
}

/*void sim_mob::Person::subscribe(sim_mob::BufferedDataManager* mgr, bool isNew) {
 Agent::subscribe(mgr, isNew); //Get x/y subscribed.
 }*/

void sim_mob::Person::buildSubscriptionList() {
	//First, add the x and y co-ordinates
	Agent::buildSubscriptionList();

	//Now, add our own properties.
	vector<BufferedBase*> roleParams = this->getRole()->getSubscriptionParams();
	for (vector<BufferedBase*>::iterator it = roleParams.begin(); it != roleParams.end(); it++) {
		subscriptionList_cached.push_back(*it);
	}
}

void sim_mob::Person::changeRole(sim_mob::Role* newRole) {
	if (this->currRole) {
		this->currRole->setParent(nullptr);
	}

	this->currRole = newRole;

	if (this->currRole) {
		this->currRole->setParent(this);
	}
}

sim_mob::Role* sim_mob::Person::getRole() const {
	return currRole;
}

#ifndef SIMMOB_DISABLE_MPI
/*
 * package Entity, Agent, Person and Role
 */
void sim_mob::Person::package(PackageUtils& packageUtil) {
	//package Entity
	//std::cout << "Person package Called" << this->getId() << std::endl;
	sim_mob::Agent::package(packageUtil);

	//package person
	packageUtil.packageBasicData(specialStr);
	packageUtil.packageTripChain(currTripChain);
}

void sim_mob::Person::unpackage(UnPackageUtils& unpackageUtil) {

	sim_mob::Agent::unpackage(unpackageUtil);
	//std::cout << "Person unpackage Called" << this->getId() << std::endl;

	specialStr = unpackageUtil.unpackageBasicData<std::string> ();
	currTripChain = const_cast<sim_mob::TripChain*>(unpackageUtil.unpackageTripChain());
}

void sim_mob::Person::packageProxy(PackageUtils& packageUtil) {
	//package Entity
	sim_mob::Agent::package(packageUtil);

	//package person
	packageUtil.packageBasicData(specialStr);
	packageUtil.packageTripChain(currTripChain);
}

void sim_mob::Person::unpackageProxy(UnPackageUtils& unpackageUtil) {
	sim_mob::Agent::unpackage(unpackageUtil);

	specialStr = unpackageUtil.unpackageBasicData<std::string> ();
	currTripChain = const_cast<sim_mob::TripChain*>(unpackageUtil.unpackageTripChain());
}

#endif
