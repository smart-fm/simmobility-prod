//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/settings/DisableMPI.h"

#include "util/LangHelpers.hpp"
#include "entities/UpdateParams.hpp"
#include "logging/Log.hpp"
#include "logging/NullableOutputStream.hpp"

namespace sim_mob {

class Vehicle;
class Person;
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
class Driver;
class Pedestrian;
class Agent;


/**
 * A Facet is a subdivision of a Role. The Facet class contains shared functionality for each type of Facet;
 *  at the moment we only have Behavior and Movement facet subclasses. The Role class just serves as a
 *  container for these two classes. Each subclass of role (Driver, Pedestrian, Passenger, ActivityRole etc.)
 *  must contain pointers/references to respective specializations of the BehaviorFacet and MovementFacet classes.
 *
 * \note
 * Make sure that your subclasses call their parent constructors (or the parent Agent won't be set). Also
 * make sure your subclasses have virtual destructors (less essential, but allows destructor chaining).
 *
 * \author Harish Loganathan
 * \author Seth N. Hetu
 */
class Facet {
public:
	explicit Facet(sim_mob::Person* parent=nullptr) : parent(parent) {}
	virtual ~Facet() {}

	//TODO: I am not sure it's a good idea to pass through directly to the parent. Might be better to
	//      find the parent Agent from the parent Role.
	sim_mob::Person* getParent();
	void setParent(sim_mob::Person* parent);

	///Called the first time an Agent's update() method is successfully called.
	/// This will be the tick of its startTime, rounded down(?).
	virtual void frame_init() = 0;

	///Perform each frame's update tick for this Agent.
	virtual void frame_tick() = 0;

	///Generate output for this frame's tick for this Agent.
	virtual void frame_tick_output() = 0;

protected:
	///Access the Logger.
	///Note that the non-standard capitalization of this function is left in for compatibility with its previous usage as a class.
 	sim_mob::NullableOutputStream Log();

	///The owner of this role. Usually a Person, but I could see it possibly being another Agent.
	sim_mob::Person* parent;
};



/**
 * See: Facet
 *
 * \author Harish Loganathan
 */
class BehaviorFacet : public Facet {
public:
	explicit BehaviorFacet(sim_mob::Person* parentAgent=nullptr) : Facet(parentAgent) { }
	virtual ~BehaviorFacet() {}

	///NOTE: There is no resource defined in the base class BehaviorFacet. For role facets of drivers, the vehicle of the parent Role could be
	///      shared between behavior and movement facets. This getter must be overridden in the derived classes to return appropriate resource.
	virtual Vehicle* getResource() { return nullptr; }


public:
	friend class sim_mob::PartitionManager;

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil) = 0;
	virtual void unpack(UnPackageUtils& unpackageUtil) = 0;

	virtual void packProxy(PackageUtils& packageUtil) = 0;
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) = 0;
#endif
};


/**
 * See: Facet
 *
 * \author Harish Loganathan
 */
class MovementFacet : public Facet {
public:
	explicit MovementFacet(sim_mob::Person* parentAgent=nullptr) : Facet(parentAgent) { }
	virtual ~MovementFacet() {}

	virtual bool updateNearbyAgent(const sim_mob::Agent* agent,const sim_mob::Driver* other_driver) { return false; };
	virtual void updateNearbyAgent(const sim_mob::Agent* agent,const sim_mob::Pedestrian* pedestrian) {};

public:
	friend class sim_mob::PartitionManager;

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil) = 0;
	virtual void unpack(UnPackageUtils& unpackageUtil) = 0;

	virtual void packProxy(PackageUtils& packageUtil) = 0;
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) = 0;
#endif
};

} // namespace sim_mob

