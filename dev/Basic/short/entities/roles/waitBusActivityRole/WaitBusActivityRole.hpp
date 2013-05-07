/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file WaitBusActivity.hpp
 *
 * \author Yao Jin
 */

#pragma once

#include "entities/roles/Role.hpp"
#include "../short/entities/roles/passenger/Passenger.hpp"
#include "entities/UpdateParams.hpp"

namespace sim_mob
{

class BusDriver;
class BusStopAgent;
//class Passenger;
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

	bool getRegisteredFlag() { return registered; }
	void setRegisteredFlag(bool registeredFlag) { registered = registeredFlag; }
	sim_mob::BusStopAgent* getBusStopAgent() { return busStopAgent; }
	BusStop* setBusStopXY(const Node* node);//to find the nearest busstop to a node
	uint32_t getTimeOfReachingBusStop() const { return TimeOfReachingBusStop; }
	void setTimeofReachingBusStop(uint32_t time) { TimeOfReachingBusStop = time; }
	std::string getBuslineID() { return buslineid; }

public:
	int boarding_Frame;// to record the boarding_frame for each individual person
	sim_mob::BusDriver* busDriver;// indicate which busDriver
	//sim_mob::Role* nextRole;// indicate next Role

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
