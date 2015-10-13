//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include<vector>

#include "ActivityFacets.hpp"
#include "conf/settings/DisableMPI.h"
#include "entities/roles/Role.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "entities/UpdateParams.hpp"
#include "geospatial/Node.hpp"

using namespace std;

namespace sim_mob
{

class PackageUtils;
class UnPackageUtils;
class Activity;
template <class PERSON> class ActivityPerformerBehavior;
template <class PERSON> class ActivityPerformerMovement;

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
#endif

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * A Person basically does nothing in the ActivityPerformer role
 *
 * \author Harish Loganathan
 */
template<class PERSON>
class ActivityPerformer : public sim_mob::Role<PERSON>
{
public:
	ActivityPerformer(PERSON* parent, sim_mob::ActivityPerformerBehavior<PERSON>* behavior = nullptr, sim_mob::ActivityPerformerMovement<PERSON>* movement = nullptr,
			std::string roleName = std::string(), typename Role<PERSON>::Type roleType_ = Role<PERSON>::RL_ACTIVITY) :
			Role<PERSON>(parent, behavior, movement, roleName, roleType_), remainingTimeToComplete(0), location(nullptr)
	{
	}

	virtual ~ActivityPerformer()
	{
	}

	//Virtual overrides
	virtual sim_mob::Role<PERSON>* clone(PERSON* parent) const
	{
		ActivityPerformerBehavior<PERSON>* behavior = new ActivityPerformerBehavior<PERSON>();
		ActivityPerformerMovement<PERSON>* movement = new ActivityPerformerMovement<PERSON>();
		ActivityPerformer<PERSON>* activityRole = new ActivityPerformer<PERSON>(parent, behavior, movement, "activityRole");
		movement->parentActivity = activityRole;
		return activityRole;
	}

	virtual void make_frame_tick_params(timeslice now)
	{
	}
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams()
	{
		return vector<BufferedBase*>();
	}

	//getters and setters
	sim_mob::DailyTime getActivityStartTime() const
	{
		return activityStartTime;
	}

	void setActivityStartTime(sim_mob::DailyTime startTime)
	{
		activityStartTime = startTime;
	}
	sim_mob::DailyTime getActivityEndTime() const
	{
		return activityEndTime;
	}
	void setActivityEndTime(sim_mob::DailyTime endTime)
	{
		activityEndTime = endTime;
	}
	const sim_mob::Node* getLocation() const
	{
		return location;
	}
	void setLocation(const sim_mob::Node* loc)
	{
		location = loc;
	}
	int getRemainingTimeToComplete() const
	{
		return remainingTimeToComplete;
	}

	/**
	 * initializes the remaining time based on start and ent time of activity
	 */
	void initializeRemainingTime()
	{
		remainingTimeToComplete = activityEndTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime())
				- activityStartTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());
	}

	/**
	 * decrements the remaining time by tick size (bounded below by 0)
	 */
	void updateRemainingTime()
	{
		remainingTimeToComplete = std::max(0, remainingTimeToComplete - int(ConfigManager::GetInstance().FullConfig().baseGranMS()));
	}

private:
	sim_mob::DailyTime activityStartTime;
	sim_mob::DailyTime activityEndTime;
	const sim_mob::Node* location;
	int remainingTimeToComplete;

	friend class ActivityPerformerBehavior<PERSON>;
	friend class ActivityPerformerMovement<PERSON>;

	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;

#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil) = 0;
	virtual void unpack(UnPackageUtils& unpackageUtil) = 0;

	virtual void packProxy(PackageUtils& packageUtil) = 0;
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) = 0;
#endif
};

}
