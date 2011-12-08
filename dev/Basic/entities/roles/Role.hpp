/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "constants.h"
#include "util/LangHelpers.hpp"
#include "entities/Agent.hpp"
#include "entities/roles/driver/UpdateParams.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/locks.hpp"
#include "util/OutputUtil.hpp"

namespace sim_mob {

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
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
		if (parent)
		{
			dynamic_seed = parent->getId();
			LogOut("synamic_seed:" << parent->getId());
		}
		else
		{
			dynamic_seed = 123;
		}
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

	int getOwnRandomNumber()
	{
		//		boost::mutex::scoped_lock lock(m_mutex);
		int one_try = -1;
		int second_try = -2;
		int third_try = -3;
		//		int forth_try = -4;

		while (one_try != second_try || third_try != second_try)
		{
			srand(dynamic_seed);
			one_try = rand();

			srand(dynamic_seed);
			second_try = rand();

			srand(dynamic_seed);
			third_try = rand();

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

	//public:
	//	static boost::mutex m_mutex;
public:
#ifndef SIMMOB_DISABLE_MPI
	friend class sim_mob::PartitionManager;
#endif


};

}
