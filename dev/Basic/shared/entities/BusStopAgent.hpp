/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * BusStopAgent.hpp
 *
 *  Created on: 2013-3-26
 *      Author: Yao Jin
*/

#pragma once

#include "conf/settings/DisableMPI.h"

#include <vector>
#include "entities/Agent.hpp"
#include "../short/entities/roles/waitBusActivityRole/WaitBusActivity.hpp"
#include "geospatial/BusStop.hpp"
#include "buffering/Shared.hpp"

namespace sim_mob
{

//Forward declarations

class PackageUtils;
class UnPackageUtils;

class BusStopAgent  : public sim_mob::Agent
{
public:
	BusStopAgent(BusStop const & busstop, const MutexStrategy& mtxStrat, int id=-1)
		  : Agent(mtxStrat, id), busstop_(busstop){};
	///Initialize a new BusStopAgent with the given busstop and MutexStrategy.
	static void RegisterNewBusStopAgent(BusStop& busstop, const MutexStrategy& mtxStrat);

	///Returns true if we have at least one BusStopAgent.
	static bool HasBusStopAgents();

	///Place all BusController agents on to the all_agents list. This does *not* add them to Worker threads
	static void PlaceAllBusStopAgents(std::vector<sim_mob::Entity*>& agents_list);

	//find one BusStopAgent by BusStop
	static BusStopAgent* findBusStopAgentByBusStop(const BusStop* busstop);

	//find one BusStopAgent by busstop_
	static BusStopAgent* findBusStopAgentByBusStopNo(const std::string& busstopno);

	///get the basic BusStop
	BusStop const & getBusStop() const { return busstop_; }
	void setBusStopAgentNo(const std::string& busstopno) { busstopAgentno_ = busstopno; }
	const std::string& getBusStopAgentNo() const { return busstopAgentno_; }
	void registerWaitingBusActivityToBusStopAgent(WaitBusActivity* wba);// for WaitBusActivity role
	void collectWaitingAgents();

	virtual ~BusStopAgent(){}
	virtual void load(const std::map<std::string, std::string>& configProps){}
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

	typedef std::vector<BusStopAgent *> All_BusStopAgents;
	static All_BusStopAgents all_BusstopAgents_;

private:
	sim_mob::BusStop const & busstop_;
	std::string busstopAgentno_; //currently is equal to busstopno_
	TimeOfReachingBusStopPriorityQueue active_waitingBusActivities;// role sorting by time reaching at the busStopAgent
	std::vector<sim_mob::Agent*> active_WaitingAgents;// possible boarding persons


#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil){}
    virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif
};
}
