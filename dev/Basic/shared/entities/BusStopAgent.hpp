//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/settings/DisableMPI.h"

#include <vector>

#include "buffering/Shared.hpp"
#include "entities/Agent.hpp"

namespace sim_mob
{

//Forward declarations
class PackageUtils;
class UnPackageUtils;
class WorkGroup;
class WaitBusActivityRole;
class BusStop;

/**
 * \author Yao Jin
 */
class BusStopAgent  : public sim_mob::Agent
{
public:
	BusStopAgent(BusStop const & busstop, const MutexStrategy& mtxStrat, int id=-1)
		  : Agent(mtxStrat, id), busstop_(busstop){};

	//Return the number of known BusStopAgents
	static size_t AllBusStopAgentsCount();

	//Assign all bus stops to Workers in the given WorkGroup
	static void AssignAllBusStopAgents(sim_mob::WorkGroup& wg);


	///Initialize a new BusStopAgent with the given busstop and MutexStrategy.
	static void RegisterNewBusStopAgent(BusStop& busstop, const MutexStrategy& mtxStrat);

	///Returns true if we have at least one BusStopAgent.
	static bool HasBusStopAgents();

	///Place all BusStopAgents on to the all_agents list. This does *not* add them to Worker threads
	static void PlaceAllBusStopAgents(std::vector<sim_mob::Entity*>& agents_list);

	//find one BusStopAgent by BusStop
	static BusStopAgent* findBusStopAgentByBusStop(const BusStop* busstop);

	//find one BusStopAgent by busstop_
	static BusStopAgent* findBusStopAgentByBusStopNo(const std::string& busstopno);

	///get the basic BusStop
	BusStop const & getBusStop() const { return busstop_; }
	void setBusStopAgentNo(const std::string& busstopno) { busstopAgentno_ = busstopno; }// set BusStopAgentNum
	const std::string& getBusStopAgentNo() const { return busstopAgentno_; }// get BusStopAgentNum
	std::vector<sim_mob::WaitBusActivityRole*>& getBoarding_WaitBusActivities() { return boarding_WaitBusActivities; }// get the boarding queue of persons for all Buslines at this BusStopAgent
	std::vector<sim_mob::Person*>& getAlighted_Persons() { return alighted_Persons; }
	void unregisterAlightedPerons();

	virtual ~BusStopAgent(){}
	virtual void load(const std::map<std::string, std::string>& configProps){}
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

	virtual bool isNonspatial();

private:
	static std::vector<BusStopAgent *> all_BusstopAgents_;

	sim_mob::BusStop const & busstop_; // BusStop object reference
	std::string busstopAgentno_; //currently is equal to busstopno_
	std::vector<sim_mob::WaitBusActivityRole*> boarding_WaitBusActivities;// one boarding queue of persons for all Buslines(temporary, each BusDriver will construct a new queue based on this)
	std::vector<sim_mob::Person*> alighted_Persons;// one alighted queue of persons.(possibly depart the persons continuing waiting and pedestrians moving to other places


#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil){}
    virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif
};
}
