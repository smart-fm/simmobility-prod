/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "conf/settings/DisableMPI.h"

#include "util/LangHelpers.hpp"
#include "entities/Agent.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "entities/UpdateParams.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/locks.hpp"
#include "logging/Log.hpp"
#include <boost/random.hpp>
#include "DriverRequestParams.hpp"
#include "RoleFacets.hpp"

namespace sim_mob {

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * Role that a person may fulfill.
 *
 * \author Seth N. Hetu
 * \author Xu Yan
 *
 *
 * Allows Person agents to swap out roles easily,
 * without re-creating themselves or maintaining temporarily irrelevant data.
 *
 * \note
 * For now, this class is very simplistic.
 */
class Role
{
public:
	//todo: use this to register roles
	enum type
	{
		RL_DRIVER,
		RL_PEDESTRIAN,
		RL_BUSDRIVER,
		RL_ACTIVITY,
		RL_PASSENGER,
		RL_WAITBUSACTITITY,
		RL_UNKNOWN
	};

	//todo: use this to identify the type of request
	enum request
	{
		REQUEST_NONE=0,
		REQUEST_DECISION_TIME,
		REQUEST_STORE_ARRIVING_TIME
	};

	const std::string name;
	const type roleType;

public:
	//NOTE: Don't forget to call this from sub-classes!
	explicit Role(sim_mob::Agent* parent = nullptr, std::string roleName = std::string(), Role::type roleType_ = RL_UNKNOWN) :
		parent(parent), currResource(nullptr), name(roleName), roleType(roleType_), dynamic_seed(0), behaviorFacet(nullptr), movementFacet(nullptr)
	{
		//todo consider putting a runtime error for empty or zero length rolename
	}

	explicit Role(sim_mob::BehaviorFacet* behavior = nullptr, sim_mob::MovementFacet* movement = nullptr, sim_mob::Agent* parent = nullptr, std::string roleName = std::string(), Role::type roleType_ = RL_UNKNOWN) :
		parent(parent), currResource(nullptr),name(roleName), roleType(roleType_), behaviorFacet(behavior), movementFacet(movement), dynamic_seed(0)
	{
		//todo consider putting a runtime error for empty or zero length rolename
	}

	//Allow propagating destructors
	virtual ~Role() {}

	//A Role must allow for copying via prototyping; this is how the RoleFactory creates roles.
	virtual Role* clone(Person* parent) const = 0;
	std::string getRoleName()const {return name;}

	///Return a list of parameters that expect their subscriptions to be managed.
	/// Agents can append/remove this list to their own subscription list each time
	/// they change their Role.
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams() = 0;

	///Create the UpdateParams (or, more likely, sub-class) which will hold all
	///  the temporary information for this time tick.
	virtual UpdateParams& make_frame_tick_params(timeslice now) = 0;

	///Return a request list for asychronous communication.
	///  Subclasses of Role should override this method if they want to enable
	///  asynchronous communication.
	///NOTE: This function is only used by the Driver class, but it's required here
	///      due to the way we split Driver into the short-term folder.
	virtual sim_mob::DriverRequestParams getDriverRequestParams() {
		return sim_mob::DriverRequestParams();
	}

	//NOTE: Should not be virtual; this is a little hackish for now. ~Seth
	Vehicle* getResource() { return currResource; }
	void setResource(Vehicle* currResource) { this->currResource = currResource; }

	Agent* getParent()
	{
		return parent;
	}

	void setParent(Agent* parent)
	{
		this->parent = parent;
	}

	int getOwnRandomNumber(boost::mt19937& gen)
	{
		int one_try = -1;
		int second_try = -2;
		int third_try = -3;
		//		int forth_try = -4;

		while (one_try != second_try || third_try != second_try)
		{
			//TODO: I've replaced your calls to srand() and rand() (which are not
			//      thread-safe) with boost::random.
			//      This is likely to not work the way you want it to.
			//      Please read the boost::random docs. ~Seth
			boost::uniform_int<> dist(0, RAND_MAX);

			one_try = dist(gen);

			second_try = dist(gen);

			third_try = dist(gen);
		}

		dynamic_seed = one_try;
		return one_try;
	}

	BehaviorFacet* Behavior() const {
		return behaviorFacet;
	}

	MovementFacet* Movement() const {
		return movementFacet;
	}

protected:
	Agent* parent; ///<The owner of this role. Usually a Person, but I could see it possibly being another Agent.

	Vehicle* currResource; ///<Roles may hold "resources" for the current task. Expand later into multiple types.

	BehaviorFacet* behaviorFacet;
	MovementFacet* movementFacet;

	//add by xuyan
protected:
	int dynamic_seed;

	//Random number generator
	//TODO: We need a policy on who can get a generator and why.
	//boost::mt19937 gen;

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

}
