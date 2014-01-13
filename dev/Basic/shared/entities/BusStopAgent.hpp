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
		  : Agent(mtxStrat, id), busstop_(busstop), frequency_OutputHeadwayGaps(900000){};

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
	void addBuslineIdCurrReachedMSs(const std::string& buslineId, uint32_t currReachedMS) {
		buslineId_CurrReachedMSs[buslineId].push_back(currReachedMS);
	}
	void addBuslineIdPassengerCounts(const std::string& buslineId, int passengerCounts) {
		buslineId_passengerCounts[buslineId].push_back(passengerCounts);
	}
	void addBuslineIdAlightingNum(const std::string& buslineId, uint32_t alightingNum) {
		buslineId_AlightingNum[buslineId].push_back(alightingNum);
	}
	void addBuslineIdBoardingNum(const std::string& buslineId, uint32_t boardingNum) {
		buslineId_BoardingNum[buslineId].push_back(boardingNum);
	}
	void addBuslineIdBoardingAlightingSecs(const std::string& buslineId, double boardingAlightingSecs) {
		buslineId_BoardingAlightingSecs[buslineId].push_back(boardingAlightingSecs);
	}
	void addBuslineIdBusTripRunSequenceNum(const std::string& buslineId, int bustripRunSequenceNum) {
		buslineId_bustripRunSequenceNums[buslineId].push_back(bustripRunSequenceNum);
	}
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
	std::map<std::string, std::vector<uint32_t> > buslineId_HeadwayGapMSs;// for each busline, list all the time headway gaps visited by each bus for this bus stop agent
	std::map<std::string, std::vector<uint32_t> > buslineId_CurrReachedMSs;// for each busline, store all the curr visited ms of bustrips for this bus stop agent

	// new added variables
	std::map<std::string, std::vector<int> > buslineId_passengerCounts;
	std::map<std::string, std::vector<uint32_t> > buslineId_AlightingNum;
	std::map<std::string, std::vector<uint32_t> > buslineId_BoardingNum;
	std::map<std::string, std::vector<double> > buslineId_BoardingAlightingSecs;
	std::map<std::string, std::vector<int> > buslineId_bustripRunSequenceNums;

	uint32_t frequency_OutputHeadwayGaps;// default every 15min(900000ms) generate output, test 3600000(1hour)


#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil){}
    virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif
};
}
