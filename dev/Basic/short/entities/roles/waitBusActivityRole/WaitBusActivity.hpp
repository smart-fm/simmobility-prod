/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file WaitBusActivity.hpp
 *
 * \author Yao Jin
 */

#pragma once

#include "entities/BusStopAgent.hpp"
#include "entities/roles/Role.hpp"
#include "entities/UpdateParams.hpp"

namespace sim_mob
{

class Passenger;
class PackageUtils;
class UnPackageUtils;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

//Helper struct
struct WaitBusActivityUpdateParams : public sim_mob::UpdateParams {
	explicit WaitBusActivityUpdateParams(boost::mt19937& gen) : UpdateParams(gen), skipThisFrame(false) {}
	virtual ~WaitBusActivityUpdateParams() {}

	virtual void reset(timeslice now)
	{
		sim_mob::UpdateParams::reset(now);
		skipThisFrame = false;
	}

	///Used to skip the first frame; kind of hackish.
	bool skipThisFrame;

#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const WaitBusActivityUpdateParams* params);
	static void unpack(UnPackageUtils& unpackage, WaitBusActivityUpdateParams* params);
#endif
};

class WaitBusActivity : public sim_mob::Role {
public:
	WaitBusActivity(Agent* parent, std::string roleName = "waitBusActivityRole");
	virtual ~WaitBusActivity();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	void initializeRemainingTime();
	void updateRemainingTime();
	///finds the nearest busstop for the given node,As passenger origin and destination is given in terms of nodes
	bool getRegisteredFlag() { return registered; }
	void setRegisteredFlag(bool registeredFlag) { registered = registeredFlag; }
	sim_mob::BusStopAgent* getBusStopAgent() { return busStopAgent; }
	BusStop* setBusStopPos(const Node* node);

//public:
//	sim_mob::Role* roleFlag;// indicate whether it can be a passenger or not
private:
	int remainingTime;
	bool registered;// indicate whether it is registered or not
	sim_mob::DailyTime activityStartTime;
	sim_mob::DailyTime activityEndTime;
	sim_mob::BusStopAgent* busStopAgent;
	uint32_t TimeOfReachingBusStop;
	Point2D DisplayOffset;

	WaitBusActivityUpdateParams params;
	friend class PackageUtils;
	friend class UnPackageUtils;

};
}
