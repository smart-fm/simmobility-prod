/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file WaitBusActivity.hpp
 *
 * \author Yao Jin
 */

#pragma once

#include "entities/roles/Role.hpp"
#include "entities/UpdateParams.hpp"

namespace sim_mob
{

class BusStopAgent;
class Passenger;
class PackageUtils;
class UnPackageUtils;
class WaitBusActivityRole;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

//Comparison for our priority queue
struct cmp_waitbusactivity_start : public std::less<WaitBusActivityRole*> {
  bool operator() (const WaitBusActivityRole* x, const WaitBusActivityRole* y) const;
};

//C++ static constructors...
class TimeOfReachingBusStopPriorityQueue : public std::priority_queue<WaitBusActivityRole*, std::vector<WaitBusActivityRole*>, cmp_waitbusactivity_start> {
};

//Helper struct
struct WaitBusActivityRoleUpdateParams : public sim_mob::UpdateParams {
	explicit WaitBusActivityRoleUpdateParams(boost::mt19937& gen) : UpdateParams(gen), skipThisFrame(false) {}
	virtual ~WaitBusActivityRoleUpdateParams() {}

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

class WaitBusActivityRole : public sim_mob::Role {
public:
	WaitBusActivityRole(Agent* parent, std::string buslineid = "", std::string roleName = "waitBusActivityRole");
	virtual ~WaitBusActivityRole();

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
	void updateBoardingTime();
	///finds the nearest busstop for the given node,As passenger origin and destination is given in terms of nodes
	bool getRegisteredFlag() { return registered; }
	void setRegisteredFlag(bool registeredFlag) { registered = registeredFlag; }
	sim_mob::BusStopAgent* getBusStopAgent() { return busStopAgent; }
	BusStop* setBusStopPos(const Node* node);
	uint32_t getTimeOfReachingBusStop() const { return TimeOfReachingBusStop; }
	std::string getBuslineID() { return buslineid; }

//public:
//	sim_mob::Role* roleFlag;// indicate whether it can be a passenger or not
public:
	int boarding_Time;// to record the boarding_frame for each individual person

private:
	int remainingTime;
	bool registered;// indicate whether it is registered or not
	sim_mob::DailyTime activityStartTime;
	sim_mob::DailyTime activityEndTime;
	sim_mob::BusStopAgent* busStopAgent;
	uint32_t TimeOfReachingBusStop;
	std::string buslineid;
	Point2D DisplayOffset;

	WaitBusActivityRoleUpdateParams params;
	friend class PackageUtils;
	friend class UnPackageUtils;

};
}
