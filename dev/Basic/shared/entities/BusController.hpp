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

class BusStop;
class BusController : public sim_mob::Agent {
private:
	explicit BusController(int id=-1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered) : Agent(mtxStrat, id),
		nextTimeTickToStage(0)
	{}

public:
	/**
	 * Initialize a single BusController with the given start time and MutexStrategy.
	 */
	static void RegisterBusController(unsigned int startTime, const MutexStrategy& mtxStrat);

	/**
	 * Returns true if we have at least one bus controller capable of dispatching buses.
	 */
	static bool HasBusControllers();

	/**
	 * get current instance
	 */
	static BusController* GetInstance();

	/**
	 * Initialize all bus controller objects based on the parameters.
	 */
	static void InitializeAllControllers(std::set<sim_mob::Entity*>& agentList, const std::vector<sim_mob::PT_BusDispatchFreq>& dispatchFreq);

	/**
	 * Place bus controller agents on to the all_agents list.
	 */
	static void DispatchAllControllers(std::set<sim_mob::Entity*>& agentList);

	/**
	 * collect and process all requests from bus drivers
	 */
	static void CollectAndProcessAllRequests();

public:
	/**
	 * inherited function to load configurable items
	 */
	virtual void load(const std::map<std::string, std::string>& configProps){}

	/**
	 * Signals are non-spatial in nature.
	 */
	virtual bool isNonspatial() { return true; }

	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	/**
	 * handler for children request
	 */
	void handleChildrenRequest();

	/**
	 * handler for driver request
	 */
	void handleEachChildRequest(sim_mob::DriverRequestParams rParams);

	/**
	 * decide holding time when a bus arrive at bus stop
	 */
	double decisionCalculation(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisited_BusStop);// return Departure MS from Aijk, DWijk etc

	/**
	 * store real times at each bus stop for future decision
	 */
	void storeRealTimesAtEachBusStop(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, const BusStop* lastVisited_BusStop, BusStopRealTimes& realTime);

	/**
	 * decide whether current agent should be into active agents list at current time
	 */
	void addOrStashBuses(Agent* p, std::set<Entity*>& activeAgents);

	/**
	 * assign bus trip information to person so as to travel on the road
	 */
	void assignBusTripChainWithPerson(std::set<sim_mob::Entity*>& activeAgents);

	/**
	 * set bus schedule which loaded from the database.
	 */
	void setPTScheduleFromConfig(const std::vector<sim_mob::PT_BusDispatchFreq>& dispatchFreq);

	/**
	 * unregister child item from children list
	 */
	virtual void unregisteredChild(Entity* child);

protected:

	/**
	 * inherited from base class agent to initialize parameters for bus controller
	 */
	virtual bool frame_init(timeslice now);

	/**
	 * inherited from base class to update this agent
	 */
	virtual Entity::UpdateStatus frame_tick(timeslice now);

	/**
	 * inherited from base class to output result
	 */
	virtual void frame_output(timeslice now);

private:

	/**
	 * estimate holding time by scheduled-based control
	 */
	double scheduledDecision(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisited_busStop);

	/**
	 * estimate holding time by headway-based control
	 */
	double headwayDecision(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisited_busStop);

	/**
	 * estimate holding time by even headway-based control
	 */
	double evenheadwayDecision(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisited_busStop);

	/**
	 * estimate holding time by hybrid-based control
	 */
	double hybridDecision(const std::string& busLine, int trip, int sequence, double arrivalTime, double departTime, BusStopRealTimes& realTime, const BusStop* lastVisited_busStop);

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
};

}

