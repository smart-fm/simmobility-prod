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

#include "GenConfig.h"
#include "entities/roles/Role.hpp"
#include "conf/simpleconf.hpp"
#include "entities/UpdateParams.hpp"
#include "geospatial/Node.hpp"

namespace sim_mob
{

class PackageUtils;
class UnPackageUtils;

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
#endif

//Helper struct
struct ActivityPerformerUpdateParams : public sim_mob::UpdateParams {
	explicit ActivityPerformerUpdateParams(boost::mt19937& gen) : UpdateParams(gen) {}
	virtual ~ActivityPerformerUpdateParams();

	virtual void reset(frame_t frameNumber, unsigned int currTimeMS)
	{
		sim_mob::UpdateParams::reset(frameNumber, currTimeMS);
		skipThisFrame = false;
	}

	///Used to skip the first frame; kind of hackish.
	bool skipThisFrame;

#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const PedestrianUpdateParams* params);

	static void unpack(UnPackageUtils& unpackage, PedestrianUpdateParams* params);
#endif
};



/**
 * A Person in the ActivityPerformer role basically does nothing
 */
class ActivityPerformer : public sim_mob::Role {
public:
	ActivityPerformer(Agent* parent);
	virtual ~ActivityPerformer();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(frame_t frameNumber);
	virtual UpdateParams& make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	sim_mob::DailyTime getActivityEndTime() const {
		return activityEndTime;
	}

	void setActivityEndTime(sim_mob::DailyTime activityEndTime) {
		this->activityEndTime = activityEndTime;
	}

	sim_mob::DailyTime getActivityStartTime() const {
		return activityStartTime;
	}

	void setActivityStartTime(sim_mob::DailyTime activityStartTime) {
		this->activityStartTime = activityStartTime;
	}

	sim_mob::Node* getLocation() const {
		return location;
	}

	void setLocation(sim_mob::Node* location) {
		this->location = location;
	}

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
};


}
