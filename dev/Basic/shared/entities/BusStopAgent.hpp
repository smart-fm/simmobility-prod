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
		  : Agent(mtxStrat, id), busstop_(busstop) {};

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
	std::vector<sim_mob::WaitBusActivityRole*>& getBoarding_WaitBusActivities() { return boardingWaitBusActivities; }// get the boarding queue of persons for all Buslines at this BusStopAgent
	void addBuslineIdCurrReachedMSs(const std::string& buslineId, uint32_t currReachedMS) {
		buslineIdCurrReachedMSs[buslineId].push_back(currReachedMS);
	}
	void addBuslineIdPassengerCounts(const std::string& buslineId, int passengerCounts) {
		buslineIdPassengerCounts[buslineId].push_back(passengerCounts);
	}
	void addBuslineIdAlightingNum(const std::string& buslineId, uint32_t alightingNum) {
		buslineIdAlightingNum[buslineId].push_back(alightingNum);
	}
	void addBuslineIdBoardingNum(const std::string& buslineId, uint32_t boardingNum) {
		buslineIdBoardingNum[buslineId].push_back(boardingNum);
	}
	void addBuslineIdBoardingAlightingSecs(const std::string& buslineId, double boardingAlightingSecs) {
		buslineIdBoardingAlightingSecs[buslineId].push_back(boardingAlightingSecs);
	}
	void addBuslineIdBusTripRunSequenceNum(const std::string& buslineId, int bustripRunSequenceNum) {
		buslineIdBusTripRunSequenceNums[buslineId].push_back(bustripRunSequenceNum);
	}

	virtual ~BusStopAgent(){}
	virtual void load(const std::map<std::string, std::string>& configProps){}
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

	virtual bool isNonspatial();

private:
	static std::vector<BusStopAgent *> all_BusstopAgents_;
	// BusStop object reference
	sim_mob::BusStop const & busstop_;
	//currently is equal to busstopno_
	std::string busstopAgentno_;
	// one boarding queue of persons for all Buslines(temporary, each BusDriver will construct a new queue based on this)
	std::vector<sim_mob::WaitBusActivityRole*> boardingWaitBusActivities;

	// new added variables
	// for each busline, store all the curr arriving ms of bustrips at this bus stop agent
	std::map<std::string, std::vector<uint32_t> > buslineIdCurrReachedMSs;
	// for each busline, store all the passenger counts of buses arriving at this bus stop agent
	std::map<std::string, std::vector<int> > buslineIdPassengerCounts;
	// for each busline, store the passenger alighting num of bustrips at this bus stop agent
	std::map<std::string, std::vector<uint32_t> > buslineIdAlightingNum;
	// for each busline, store the passenger boarding num of bustrips at this bus stop agent
	std::map<std::string, std::vector<uint32_t> > buslineIdBoardingNum;
	// for each busline, store the boarding and alighting secs(dwell time secs) of bustrips at this bus stop agent
	std::map<std::string, std::vector<double> > buslineIdBoardingAlightingSecs;
	// for each busline, store the bus trip run sequence num of bustrips at this bus stop agent
	std::map<std::string, std::vector<int> > buslineIdBusTripRunSequenceNums;


#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil){}
    virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif
};
}
