/*
 *
 * Copyright Singapore-MIT Alliance for Research and Technology
 *
 * Household.hpp
 *
 *  Created on: Mar 5, 2013
 *      Author: gandola
 */

#pragma once

#include "conf/settings/DisableMPI.h"

#include <vector>
#include <map>
#include <string>
#include "entities/roles/Role.hpp"
#include "event/EventListener.hpp"

namespace sim_mob {

class Agent;
class Person;
class UpdateParams;
#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

using std::vector;
using std::map;
using std::string;

namespace long_term {

class Household: public Role, public EventListener {
	/*
	 * Methods
	 */
public:
	Household(Agent* parent);
	virtual ~Household();

	//Inherited from sim_mob::Role
	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
        virtual void OnEvent(EventPublisher* sender, EventId id, const EventArgs& args);
#ifndef SIMMOB_DISABLE_MPI
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);
	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif


	/*
	 * Fields
	 */
public:
	static const string ROLE_NAME;
private:
	boost::mt19937 gen; // only for testing.
	UpdateParams params;
};
} /* namespace long_term */

} /* namespace sim_mob */
