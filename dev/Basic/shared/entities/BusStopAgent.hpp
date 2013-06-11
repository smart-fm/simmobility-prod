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
	static void RegisterNewBusStopAgent(BusStop const & busstop, const MutexStrategy& mtxStrat, int id=-1);

	///Returns true if we have at least one BusStopAgent.
	static bool HasBusStopAgents();

	///Place all BusController agents on to the all_agents list. This does *not* add them to Worker threads
	static void PlaceAllBusStopAgents(std::vector<sim_mob::Entity*>& agents_list);

	///get the basic BusStop
	BusStop const & getBusStop() const { return busstop_; }

	virtual ~BusStopAgent(){}
	virtual void load(const std::map<std::string, std::string>& configProps){}
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

	virtual bool isNonspatial();

	typedef std::vector<BusStopAgent *> All_BusStopAgents;
	static All_BusStopAgents all_BusstopAgents_;
private:
	sim_mob::BusStop const & busstop_;


#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil){}
    virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif
};
}
