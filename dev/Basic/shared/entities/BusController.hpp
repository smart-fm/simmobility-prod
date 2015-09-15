//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/settings/DisableMPI.h"

#include <set>
#include <vector>

#include "buffering/Shared.hpp"
#include "entities/Agent.hpp"
#include "misc/PublicTransit.hpp"
#include "roles/DriverRequestParams.hpp"
#include "util/DynamicVector.hpp"

namespace sim_mob {

class Bus;
class BusStop;

/*
 * BusController class.
 * \author Yao Jin
 * \refactor by zhang huai peng
 */
class BusController : public sim_mob::Agent {
private:
	explicit BusController(int id=-1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered) : Agent(mtxStrat, id),
		nextTimeTickToStage(0)
	{}

public:
	///Initialize a single BusController with the given start time and MutexStrategy.
	static void RegisterBusController(unsigned int startTime, const MutexStrategy& mtxStrat);

	///Returns true if we have at least one bus controller capable of dispatching buses.
	static bool HasBusControllers();

	///get current instance
	static BusController* GetInstance();

	///Initialize all bus controller objects based on the parameters.
	static void InitializeAllControllers(std::set<sim_mob::Entity*>& agentList, const std::vector<sim_mob::PT_bus_dispatch_freq>& dispatchFreq);

	///Place all BusController agents on to the all_agents list.
	static void DispatchAllControllers(std::set<sim_mob::Entity*>& agentList);

	///collect and process all requests from bus drivers
	static void CollectAndProcessAllRequests();

public:
	//May implement later
	virtual void load(const std::map<std::string, std::string>& configProps){}

	//Signals are non-spatial in nature.
	virtual bool isNonspatial() { return true; }

	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	void handleChildrenRequest();
	void handleRequestParams(sim_mob::DriverRequestParams rParams);

	double decisionCalculation(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStop_RealTimes& realTime, const BusStop* lastVisited_BusStop);// return Departure MS from Aijk, DWijk etc
	void storeRealTimesAtEachBusStop(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, const BusStop* lastVisited_BusStop, BusStop_RealTimes& realTime);
	void addOrStashBuses(Agent* p, std::set<Entity*>& activeAgents);

	void assignBusTripChainWithPerson(std::set<sim_mob::Entity*>& activeAgents);

	///Load all bus items from the database.
	void setPTScheduleFromConfig(const std::vector<sim_mob::PT_bus_dispatch_freq>& dispatchFreq);

	virtual void unregisteredChild(Entity* child);

protected:
	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

private:
	double scheduledDecision(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStop_RealTimes& realTime, const BusStop* lastVisited_busStop);// scheduled-based control
	double headwayDecision(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStop_RealTimes& realTime, const BusStop* lastVisited_busStop); // headway-based control
	double evenheadwayDecision(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStop_RealTimes& realTime, const BusStop* lastVisited_busStop); // evenheadway-based control
	double hybridDecision(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStop_RealTimes& realTime, const BusStop* lastVisited_busStop); // hybrid-based control(evenheadway while restricting the maximum holding time)

	/**
	 * record next time tick to help dispatching decision
	 */
	uint32_t nextTimeTickToStage;
	/**
	 * buses waiting to be added to the simulation, prioritized by start time.
	 */
	StartTimePriorityQueue pendingChildren;
	/**
	 * hold bus schedule information
	 */
	PT_Schedule ptSchedule;
	/**
	 * reference to the instance of bus controller
	 */
	static BusController* instance;
	/**
	 * keep all children agents to communicate with it
	 */
	std::vector<Entity*> allChildren;

#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil);
    virtual void unpack(UnPackageUtils& unpackageUtil);

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif
};

}

