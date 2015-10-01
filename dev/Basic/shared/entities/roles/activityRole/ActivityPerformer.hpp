//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file ActivityPerformer.hpp
 *
 * \author Harish
 */

#pragma once

#include "ActivityFacets.hpp"
#include "conf/settings/DisableMPI.h"
#include "entities/roles/Role.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "entities/UpdateParams.hpp"
#include "geospatial/Node.hpp"

namespace sim_mob
{

class PackageUtils;
class UnPackageUtils;
class Activity;
class ActivityPerformerBehavior;
class ActivityPerformerMovement;

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
#endif

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * A Person basically does nothing in the ActivityPerformer role
 */
class ActivityPerformer : public sim_mob::Role<Person>
{
public:
	ActivityPerformer(sim_mob::Person* parent, sim_mob::ActivityPerformerBehavior* behavior = nullptr, sim_mob::ActivityPerformerMovement* movement = nullptr, std::string roleName = std::string(), Role::Type roleType_ = RL_ACTIVITY);
	ActivityPerformer(sim_mob::Person* parent, const sim_mob::Activity& currActivity, sim_mob::ActivityPerformerBehavior* behavior = nullptr, sim_mob::ActivityPerformerMovement* movement = nullptr, Role::Type roleType_ = RL_ACTIVITY, std::string roleName = "activityRole");
	virtual ~ActivityPerformer();

	//Virtual overrides
	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;
	virtual void make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	//getters and setters
	sim_mob::DailyTime getActivityStartTime() const;
	void setActivityStartTime(sim_mob::DailyTime activityStartTime);
	sim_mob::DailyTime getActivityEndTime() const;
	void setActivityEndTime(sim_mob::DailyTime activityEndTime);
	sim_mob::Node* getLocation() const;
	void setLocation(sim_mob::Node* location);
	int getRemainingTimeToComplete() const;

	/**
	 * initializes the remaining time based on start and ent time of activity
	 */
	void initializeRemainingTime();

	/**
	 * decrements the remaining time by tick size (bounded below by 0)
	 */
	void updateRemainingTime();

private:
	sim_mob::DailyTime activityStartTime;
	sim_mob::DailyTime activityEndTime;
	sim_mob::Node* location;
	int remainingTimeToComplete;

	friend class ActivityPerformerBehavior;
	friend class ActivityPerformerMovement;

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
