/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * BusController.hpp
 *
 *  Created on: 2012-6-11
 *      Author: Yao Jin
 */

#pragma once

#include <vector>

#include "entities/Agent.hpp"

#include "buffering/Shared.hpp"
#include "entities/UpdateParams.hpp"
#include "vehicle/Bus.hpp"
#include "roles/driver/Driver.hpp"
#include "util/DynamicVector.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"

namespace sim_mob
{
class Bus;

class BusController : public sim_mob::Agent
{
public:
	//Note: I am making this a pointer, since the all_agents array is now cleared and deleted on exit.
	//      Otherwise, it will attempt to delete itself twice.
	static BusController* busctrller;

	~BusController();
	virtual Entity::UpdateStatus update(frame_t frameNumber);
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);
	void updateBusInformation(DPoint pt);
	void addOrStashBuses(const PendingEntity& p, std::vector<Entity*>& active_agents);
	//void DispatchInit();// similar to AddandStash --> p.start = 0

	//NOTE: There's two problems here:
	//      1) You use a static "BusController", which is not flexible.
	//      2) You use a flag "isToBeInList" to determine if output should be produced each time tick.
	//The proper way to do this is to have BusController(s) load from the config file and NOT be static,
	//      and then always assume that output will be printed. Remember, we are using an agent-based
	//      system, so the idea of a "static" agent doesn't make a lot of sense.
	//For now, I am fixing this by having getToBeInList() always return true.
	bool getToBeInList() { return true; }

	// Manage Buses
	void addBus(Bus* bus);
	void remBus(Bus* bus);
	///Retrieve a reference to the list of managedBuses.
	//std::vector<Bus*>& getManagedBuses() { return managedBuses; }
	//const sim_mob::RoadNetwork& getNetwork() { return network; }

private:
	explicit BusController(int id=-1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered);

	void DispatchFrameTick(frame_t frameTick);
	void frame_init(frame_t frameNumber);
	void frame_tick_output(frame_t frameNumber);
	//static BusController instance_;

	frame_t frameNumberCheck;// check some frame number to do control
	frame_t nextTimeTickToStage;// next timeTick to be checked
	unsigned int tickStep;
	bool firstFrameTick;  ///Determines if frame_init() has been done.
	std::vector<Bus*> managedBuses;// Saved all virtual managedBuses
	//std::vector<Bus*> active_buses;// Saved all active buses(from frame 0)
	StartTimePriorityQueue pending_buses; //Buses waiting to be added to the simulation, prioritized by start time.
	//sim_mob::RoadNetwork network;// Saved RoadNetwork
	DPoint posBus;// The sent position of a given bus ,only for test

#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil){}
    virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif
};

}

