/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "GenConfig.h"
#include "util/LangHelpers.hpp"
#include "entities/Agent.hpp"
#include "entities/roles/driver/DriverUpdateParams.hpp"
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

	int getOwnRandomNumber(boost::mt19937& gen)
	{
		//		boost::mutex::scoped_lock lock(m_mutex);
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

//			if (one_try != second_try || third_try != second_try)
//			{
//				LogOut("Random:" << this->getParent()->getId() << "," << one_try << "," << second_try << "," << third_try << "\n");
//			}
//			else
//			{
//				LogOut("Random:" << this->getParent()->getId() << ",Use Seed:" << dynamic_seed << ", Get:" << one_try << "," << second_try<< "," << third_try<< "\n");
//			}
		}

		dynamic_seed = one_try;
		return one_try;
	}

protected:
	Agent* parent; ///<The owner of this role. Usually a Person, but I could see it possibly being another Agent.

	//add by xuyan
protected:
	int dynamic_seed;

	//Random number generator
	//TODO: We need a policy on who can get a generator and why.
	//boost::mt19937 gen;

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
