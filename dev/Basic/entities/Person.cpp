/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Person.hpp"

//For debugging
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "util/DebugFlags.hpp"

using std::vector;
using namespace sim_mob;

sim_mob::Person::Person(const MutexStrategy& mtxStrat, int id) :
	Agent(mtxStrat, id), currRole(nullptr), currTripChain(nullptr), firstFrameTick(true) {

}

sim_mob::Person::~Person() {
	safe_delete(currRole);
}

bool sim_mob::Person::update(frame_t frameNumber) {
	//First, we need to retrieve an UpdateParams subclass appropriate for this Agent.
	unsigned int currTimeMS = frameNumber * ConfigParams::GetInstance().baseGranMS;
	UpdateParams& params = currRole->make_frame_tick_params(frameNumber, currTimeMS);

	//Has update() been called early?
	if(currTimeMS < getStartTime()) {
		//This only represents an error if dynamic dispatch is enabled. Else, we silently skip this update.
		if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			std::stringstream msg;
			msg << "Agent(" << getId() << ") specifies a start time of: " << getStartTime()
					<< " but it is currently: " << currTimeMS
					<< "; this indicates an error, and should be handled automatically.";
			throw std::runtime_error(msg.str().c_str());
		}
		return true;
	}

	//Has update() been called too late?
	if (isToBeRemoved()) {
		//This only represents an error if dynamic dispatch is enabled. Else, we silently skip this update.
		if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			throw std::runtime_error("Agent is already done, but hasn't been removed.");
		}
		return true;
	}

	//Is this the first frame tick for this Agent?
	if (firstFrameTick) {
		//Helper check; not needed once we trust our Workers.
		if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			if (abs(currTimeMS-getStartTime())>=ConfigParams::GetInstance().baseGranMS) {
				std::stringstream msg;
				msg << "Agent was not started within one timespan of its requested start time.";
				msg << "\nStart was: " << getStartTime() << ",  Curr time is: " << currTimeMS << "\n";
				msg << "Agent ID: " << getId() << "\n";
				throw std::runtime_error(msg.str().c_str());
			}
		}

		currRole->frame_init(params);
		firstFrameTick = false;
	}

	//Now perform the main update tick
	if (!isToBeRemoved()) {
		currRole->frame_tick(params);
	}

	//Finally, save the output
	if (!isToBeRemoved()) {
		currRole->frame_tick_output(params);
	}

	//Output if removal requested.
	if (Debug::WorkGroupSemantics && isToBeRemoved()) {
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout << "Person requested removal: " << (dynamic_cast<Driver*> (currRole) ? "Driver"
				: dynamic_cast<Pedestrian*> (currRole) ? "Pedestrian" : "Other") << "\n";
	}

	//Return true unless we are scheduled for removal.
	//NOTE: Make sure you set this flag AFTER performing your final output.
	return !isToBeRemoved();
}

/*void sim_mob::Person::output(frame_t frameNumber) {
	if (currRole) {
		currRole->output(frameNumber);
	}
}*/

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
