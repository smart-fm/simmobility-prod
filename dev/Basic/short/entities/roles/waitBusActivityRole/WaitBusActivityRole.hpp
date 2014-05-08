//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file WaitBusActivity.hpp
 *
 * \author Yao Jin
 */

#pragma once

#include "entities/roles/Role.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "WaitBusActivityRoleFacets.hpp"

namespace sim_mob
{
class BusDriver;
class PackageUtils;
class UnPackageUtils;
class WaitBusActivityRoleBehavior;
class WaitBusActivityRoleMovement;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

//Helper struct
struct WaitBusActivityRoleUpdateParams : public sim_mob::UpdateParams {

	WaitBusActivityRoleUpdateParams() : UpdateParams() {}
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

class WaitBusActivityRole : public sim_mob::Role , public UpdateWrapper<WaitBusActivityRoleUpdateParams>{
public:
	WaitBusActivityRole(Agent* parent, sim_mob::WaitBusActivityRoleBehavior* behavior = nullptr, sim_mob::WaitBusActivityRoleMovement* movement = nullptr, Role::type roleType_ = RL_WAITBUSACTITITY, std::string roleName = "waitBusActivityRole");
	virtual ~WaitBusActivityRole();
	virtual void make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

//	bool getRegisteredFlag() { return registered; } // get the registered flag
//	void setRegisteredFlag(bool registeredFlag) { registered = registeredFlag; } // set the registered flag
//	sim_mob::BusStopAgent* getBusStopAgent() { return busStopAgent; }
//	BusStop* setBusStopXY(const Node* node);//to find the nearest busstop to a node
	uint32_t getTimeOfReachingBusStop() const { return TimeOfReachingBusStop; }
	uint32_t getWaitingTimeAtBusStop() const {
		return waitingTimeAtBusStop;
	}

public:
	uint32_t TimeOfReachingBusStop;
	uint32_t waitingTimeAtBusStop;

private:
	WaitBusActivityRoleUpdateParams params;
	friend class PackageUtils;
	friend class UnPackageUtils;
};

struct less_than_TimeOfReachingBusStop {
	inline bool operator() (const WaitBusActivityRole* wbaRole1, const WaitBusActivityRole* wbaRole2)
	{
		if ((!wbaRole1) || (!wbaRole2)) {
			return 0;
		}

		return (wbaRole1->getTimeOfReachingBusStop() < wbaRole2->getTimeOfReachingBusStop());
	}
};
}
