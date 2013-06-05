/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file ActivityPerformer.hpp
 *
 * \author Harish
 */

#pragma once

#include <time.h>
#include <math.h>

#include <boost/random.hpp>

#include "conf/settings/DisableMPI.h"

#include "entities/roles/Role.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "ActivityFacets.hpp"
#include "conf/simpleconf.hpp"
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

//Helper struct
struct ActivityPerformerUpdateParams : public sim_mob::UpdateParams {
	explicit ActivityPerformerUpdateParams(boost::mt19937& gen);
	virtual ~ActivityPerformerUpdateParams() {}

	virtual void reset(timeslice now)
	{
		sim_mob::UpdateParams::reset(now);
		skipThisFrame = false;
	}

	///Used to skip the first frame; kind of hackish.
	bool skipThisFrame;

#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const ActivityPerformerUpdateParams* params);

	static void unpack(UnPackageUtils& unpackage, ActivityPerformerUpdateParams* params);
#endif
};

/**
 * A Person basically does nothing in the ActivityPerformer role
 */
class ActivityPerformer : public sim_mob::Role {
public:
	int remainingTimeToComplete;

	ActivityPerformer(sim_mob::Agent* parent, sim_mob::ActivityPerformerBehavior* behavior = nullptr, sim_mob::ActivityPerformerMovement* movement = nullptr, std::string roleName = std::string());
	ActivityPerformer(sim_mob::Agent* parent, const sim_mob::Activity& currActivity, sim_mob::ActivityPerformerBehavior* behavior = nullptr, sim_mob::ActivityPerformerMovement* movement = nullptr, std::string roleName = "activityRole");
	virtual ~ActivityPerformer() {}

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual void frame_tick_output_mpi(timeslice now);
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	sim_mob::DailyTime getActivityEndTime() const;
	void setActivityEndTime(sim_mob::DailyTime activityEndTime);
	sim_mob::DailyTime getActivityStartTime() const;
	void setActivityStartTime(sim_mob::DailyTime activityStartTime);
	sim_mob::Node* getLocation() const;
	void setLocation(sim_mob::Node* location);
	void initializeRemainingTime();
	void updateRemainingTime();

private:
	sim_mob::DailyTime activityStartTime;
	sim_mob::DailyTime activityEndTime;
	sim_mob::Node* location;

	//Temporary variable which will be flushed each time tick. We save it
	// here to avoid constantly allocating and clearing memory each time tick.
	ActivityPerformerUpdateParams params;
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

	friend class ActivityPerformerBehavior;
	friend class ActivityPerformerMovement;
};

}
