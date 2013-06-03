/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file WaitBusActivity.hpp
 *
 * \author Yao Jin
 */

#pragma once

#include "entities/roles/Role.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/BusStopAgent.hpp"

namespace sim_mob
{
class BusDriver;
class PackageUtils;
class UnPackageUtils;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

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
	WaitBusActivityRole(Agent* parent, Role::type roleType_ = RL_WAITBUSACTITITY, std::string buslineid = "", std::string roleName = "waitBusActivityRole");
	virtual ~WaitBusActivityRole();
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	bool getRegisteredFlag() { return registered; } // get the registered flag
	void setRegisteredFlag(bool registeredFlag) { registered = registeredFlag; } // set the registered flag
	sim_mob::BusStopAgent* getBusStopAgent() { return busStopAgent; }
	BusStop* setBusStopXY(const Node* node);//to find the nearest busstop to a node
	uint32_t getTimeOfReachingBusStop() const { return TimeOfReachingBusStop; }
	//void setTimeofReachingBusStop(uint32_t time) { TimeOfReachingBusStop = time; }
	std::string getBuslineID() { return buslineid; }

public:
    //Set by the BusDriver to the MS this Person should board the bus.
	uint32_t boarding_MS;
	//Indicates the BusDriver of the bus we will board when "boarding_Frame" is reached.
	sim_mob::BusDriver* busDriver;
protected:
	//Indicates whether or not this Activity has registered with the appropriate BusStopAgent.
	//An unregistered Activity will not be able to board buses.
	//BusStopAgents will automatically register every WaitBusActivityRole in their vicinity periodically.
	bool registered;
	sim_mob::BusStopAgent* busStopAgent;
	uint32_t TimeOfReachingBusStop;
	std::string buslineid;

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
