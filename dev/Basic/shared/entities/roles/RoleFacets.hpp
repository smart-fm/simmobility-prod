/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * RoleFacet.h
 *
 *  Created on: Mar 21, 2013
 *      Author: harish
 */

#pragma once

#include "conf/settings/DisableMPI.h"
#include "util/LangHelpers.hpp"
#include "entities/Agent.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/BusTrip.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/locks.hpp"
#include "util/OutputUtil.hpp"
#include <boost/random.hpp>

namespace sim_mob {

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
#endif

/* The BehaviorFacet and MovementFacet abstract base classes are pretty much identical. They are kept separate for semantic reasons.
 * The Role class will just serve as a container for these two classes. Each subclass of role (Driver, Pedestrian, Passenger, ActivityRole etc.)
 * must contain pointers/references to respective specializations of BehaviorFacet and MovementFacet classes.
 */

class BehaviorFacet {

public:
	//NOTE: Don't forget to call this from sub-classes!
	explicit BehaviorFacet(sim_mob::Person* parentAgent = nullptr) :
		parentAgent(parentAgent) { }

	//Allow propagating destructors
	virtual ~BehaviorFacet() {}

	///Called the first time an Agent's update() method is successfully called.
	/// This will be the tick of its startTime, rounded down(?).
	virtual void frame_init(UpdateParams& p) = 0;

	///Perform each frame's update tick for this Agent.
	virtual void frame_tick(UpdateParams& p) = 0;

	///Generate output for this frame's tick for this Agent.
	virtual void frame_tick_output(const UpdateParams& p) = 0;

	//generate output with fake attributes for MPI
	virtual void frame_tick_output_mpi(timeslice now) = 0;

	/* NOTE: There is no resource defined in the base class BehaviorFacet. For role facets of drivers, the vehicle of the parent Role could be
	 * shared between behavior and movement facets. This getter must be overridden in the derived classes to return appropriate resource.
	 */
	virtual Vehicle* getResource() { return nullptr; }

	Person* getParent()
	{
		return parentAgent;
	}

	void setParent(Person* parent)
	{
		this->parentAgent = parent;
	}

protected:
	Person* parentAgent; ///<The owner of this role. Usually a Person, but I could see it possibly being another Agent.

public:
#ifndef SIMMOB_DISABLE_MPI
	friend class sim_mob::PartitionManager;
#endif

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil) = 0;
	virtual void unpack(UnPackageUtils& unpackageUtil) = 0;

	virtual void packProxy(PackageUtils& packageUtil) = 0;
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) = 0;
#endif
};

class MovementFacet {

public:
	//NOTE: Don't forget to call this from sub-classes!
	explicit MovementFacet(sim_mob::Person* parentAgent = nullptr) :
		parentAgent(parentAgent) { }

	//Allow propagating destructors
	virtual ~MovementFacet() {}

	///Called the first time an Agent's update() method is successfully called.
	/// This will be the tick of its startTime, rounded down(?).
	virtual void frame_init(UpdateParams& p) = 0;

	///Perform each frame's update tick for this Agent.
	virtual void frame_tick(UpdateParams& p) = 0;

	///Generate output for this frame's tick for this Agent.
	virtual void frame_tick_output(const UpdateParams& p) = 0;

	//generate output with fake attributes for MPI
	virtual void frame_tick_output_mpi(timeslice now) = 0;

	//for use by confluxes to permit the person to move to next link
	virtual void flowIntoNextLinkIfPossible(UpdateParams& p) = 0;

	Person* getParent()
	{
		return parentAgent;
	}

	void setParent(Person* parent)
	{
		this->parentAgent = parent;
	}

protected:
	Person* parentAgent; ///<The owner of this role. Usually a Person, but I could see it possibly being another Agent.

public:
#ifndef SIMMOB_DISABLE_MPI
	friend class sim_mob::PartitionManager;
#endif

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil) = 0;
	virtual void unpack(UnPackageUtils& unpackageUtil) = 0;

	virtual void packProxy(PackageUtils& packageUtil) = 0;
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) = 0;
#endif
};

} /* namespace sim_mob */
