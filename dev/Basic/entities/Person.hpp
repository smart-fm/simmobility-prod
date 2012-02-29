/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "GenConfig.h"

#include "Agent.hpp"
#include "roles/Role.hpp"
#include "roles/driver/Driver.hpp"
#include "buffering/Shared.hpp"
#include "entities/UpdateParams.hpp"

namespace sim_mob
{

class TripChain;

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * Basic Person class.
 *
 * \author Seth N. Hetu
 * \author Wang Xinyuan
 * \author Luo Linbo
 * \author Li Zhemin
 * \author Xu Yan
 *
 * A person may perform one of several roles which
 *  change over time. For example: Drivers, Pedestrians, and Passengers are
 *  all roles which a Person may fulfill.
 */
class Person : public sim_mob::Agent {
public:
	explicit Person(const MutexStrategy& mtxStrat, int id=-1);
	virtual ~Person();

	///Generate a person from a PendingEntity. Currently only works for Drivers/Pedestrians
	static Person* GeneratePersonFromPending(const PendingEntity& p);

	///Update Person behavior
	virtual Entity::UpdateStatus update(frame_t frameNumber) final;

	///Update a Person's subscription list.
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	///Change the role of this person: Driver, Passenger, Pedestrian
	void changeRole(sim_mob::Role* newRole);
	sim_mob::Role* getRole() const;

	///Check if any role changing is required.
	Entity::UpdateStatus checkAndReactToTripChain(unsigned int currTimeMS);


	///Set this person's trip chain
	///
	///\todo
	///Currently, there are two types of trip chains. Type 1 is from the database, and
	///  can be shared among multiple Agents. Type 2 is created "ad hoc" for a single Agent.
	///  Creating the second type of TripChain will leak memory when the Person is remoed
	///  from the simulation. This is exactly the kind of thing which std::shared_pointer
	///  is good for. Maybe we should use the boost:: version?
	void setTripChain(sim_mob::TripChain* newTripChain) { currTripChain = newTripChain; }
	sim_mob::TripChain* getTripChain() { return currTripChain; }

	//Used for passing various debug data. Do not rely on this for anything long-term.
	std::string specialStr;

private:
	//Properties
	sim_mob::Role* prevRole;  ///< To be deleted on the next time tick.
	sim_mob::Role* currRole;
	sim_mob::TripChain* currTripChain;

	bool firstFrameTick;  ///Determines if frame_init() has been done.

	//add by xuyan
#ifndef SIMMOB_DISABLE_MPI
public:
	friend class PartitionManager;
	friend class BoundaryProcessor;

public:
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);

#endif

};





}
