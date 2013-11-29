//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/settings/DisableMPI.h"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/roles/pedestrian/Pedestrian2.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "PassengerFacets.hpp"

namespace sim_mob
{

/**
 * A Person in the Passenger role is likely just waiting for his or her bus stop.
 * \author Meenu
 */
//class BusDriver;
class BusStop;
class Person;
class Bus;
class Passenger;
class PassengerBehavior;
class PassengerMovement;
class PackageUtils;
class UnPackageUtils;

struct PassengerUpdateParams : public sim_mob::UpdateParams {
	PassengerUpdateParams() : UpdateParams() {}
	explicit PassengerUpdateParams(boost::mt19937& gen) : UpdateParams(gen) {}
	virtual ~PassengerUpdateParams() {}

	virtual void reset(timeslice now)
	{
		sim_mob::UpdateParams::reset(now);
	}

#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const PassengerUpdateParams* params);
	static void unpack(UnPackageUtils& unpackage, PassengerUpdateParams* params);
#endif
};


class Passenger : public sim_mob::Role , public UpdateWrapper<PassengerUpdateParams>{
public:
	Passenger(Agent* parent, MutexStrategy mtxStrat, sim_mob::PassengerBehavior* behavior = nullptr, sim_mob::PassengerMovement* movement = nullptr, Role::type roleType_ = RL_PASSENGER, std::string roleName_ = "passenger");
	virtual ~Passenger() {}

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;
	void make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	const uint32_t getWaitingTimeAtStop() const {
		return waitingTimeAtStop;
	}
	void setWaitingTimeAtStop(uint32_t waitingTime) {
		waitingTimeAtStop = waitingTime;
	}

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil){}
	virtual void unpack(UnPackageUtils& unpackageUtil){}
	virtual void packProxy(PackageUtils& packageUtil){}
	virtual void unpackProxy(UnPackageUtils& unpackageUtil){}
#endif

public:
	//uint32_t alighting_MS;// to record the alighting_MS for each individual person
	sim_mob::Shared<Driver*> busdriver;///passenger should have info about the driver
	sim_mob::Shared<bool> BoardedBus;
	sim_mob::Shared<bool> AlightedBus;
	uint32_t waitingTimeAtStop;
private:
	PassengerUpdateParams params;
};



}
