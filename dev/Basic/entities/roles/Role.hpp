/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "constants.h"
#include "util/LangHelpers.hpp"
#include "entities/Agent.hpp"
#include "entities/roles/driver/UpdateParams.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/locks.hpp"
#include "util/OutputUtil.hpp"

//#ifndef SIMMOB_DISABLE_MPI
//#include "partitions/PackageUtils.hpp"
//#include "partitions/UnPackageUtils.hpp"
//#endif

namespace sim_mob {

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * Role that a person may fulfill. Allows Person agents to swap out roles easily,
 * without re-creating themselves or maintaining temporarily irrelevant data.
 *
 * \note
 * For now, this class is very simplistic.
 */
class Role
{
public:
	//NOTE: Don't forget to call this from sub-classes!
	Role(Agent* parent = nullptr) :
		parent(parent)
	{
	}

	/// TODO: Think through what kind of data this function might need.
	/// Frame number? Elapsed time?
	virtual void update(frame_t frameNumber) = 0;

	virtual void output(frame_t frameNumber) = 0;

	///Return a list of parameters that expect their subscriptions to be managed.
	/// Agents can append/remove this list to their own subscription list each time
	/// they change their Role.
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams() = 0;

	Agent* getParent()
	{
		return parent;
	}

	void setParent(Agent* parent)
	{
		this->parent = parent;
	}

protected:
	Agent* parent; ///<The owner of this role. Usually a Person, but I could see it possibly being another Agent.

	//public:
	//	static boost::mutex m_mutex;
public:
#ifndef SIMMOB_DISABLE_MPI
	friend class sim_mob::PartitionManager;
#endif

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void package(PackageUtils& packageUtil) = 0;
	virtual void unpackage(UnPackageUtils& unpackageUtil) = 0;

	virtual void packageProxy(PackageUtils& packageUtil) = 0;
	virtual void unpackageProxy(UnPackageUtils& unpackageUtil) = 0;
#endif


};

}
