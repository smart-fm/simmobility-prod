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
#include "misc/BusTrip.hpp"
#include "vehicle/Bus.hpp"
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
	//static BusController* busctrller;
	static std::vector<BusController*> all_busctrllers_;
	static void registerBusController(unsigned int startTime, const MutexStrategy& mtxStrat);

	~BusController();
	virtual Entity::UpdateStatus update(frame_t frameNumber);
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	void receiveBusInformation(int busline_i = 1, int trip_k = 0, int busstopSequence_j = 0, unsigned int ATijk = 0);
	// offsetMS_From(ConfigParams::GetInstance().simStartTime))???
	unsigned int decisionCalculation(int busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk, int lastVisited_BusStopSeqNum);// return Departure MS from Aijk, DWijk etc
	unsigned int sendBusInformation();// depend on the control strategy
	void addOrStashBuses(Agent* p, std::vector<Entity*>& active_agents);

	//May implement later
	virtual void load(const std::map<std::string, std::string>& configProps){}

	//NOTE: There's two problems here:
	//      1) You use a static "BusController", which is not flexible.
	//      2) You use a flag "isToBeInList" to determine if output should be produced each time tick.
	//The proper way to do this is to have BusController(s) load from the config file and NOT be static,
	//      and then always assume that output will be printed. Remember, we are using an agent-based
	//      system, so the idea of a "static" agent doesn't make a lot of sense.
	//For now, I am fixing this by having getToBeInList() always return true.
	bool getToBeInList() { return true; }

	//Functions required by Jenny's code.
	// TODO: These shouldn't have to be duplicated across all entity types.
	virtual Link* getCurrLink();
	virtual void setCurrLink(Link* link);

	// Manage Buses
	void addBus(Bus* bus);
	void remBus(Bus* bus);
	bool SetRouteforBusTrip(unsigned int busRoute_id);
	sim_mob::BusTrip* MakeBusTrip(const TripChainItem& tcItem);
	void assignBusTripChainWithPerson();

private:
	explicit BusController(int id=-1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered);

	void dispatchFrameTick(frame_t frameTick);
	void frame_init(frame_t frameNumber);
	void frame_tick_output(frame_t frameNumber);

	unsigned int scheduledDecision(int busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk);// scheduled-based control
	unsigned int headwayDecision(int busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk); // headway-based control
	unsigned int evenheadwayDecision(int busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk, int lastVisited_BusStopSeqNum); // evenheadway-based control
	unsigned int hybridDecision(int busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk); // hybrid-based control(evenheadway while restricting the maximum holding time)
	unsigned int dwellTimeCalculation(int busline_i, int trip_k, int busstopSequence_j); // dwell time calculation module

	frame_t frameNumberCheck;// check some frame number to do control
	frame_t nextTimeTickToStage;// next timeTick to be checked
	unsigned int tickStep;
	bool firstFrameTick;  ///Determines if frame_init() has been done.
	std::vector<Bus*> managedBuses;// Saved all virtual managedBuses
	StartTimePriorityQueue pending_buses; //Buses waiting to be added to the simulation, prioritized by start time.
	DPoint posBus;// The sent position of a given bus ,only for test

	PT_Schedule pt_schedule;
	//The current Link. Used by Jenny's code (except we don't currently use buses in the medium term)
    sim_mob::Link* currLink;

    //std::map<int, std::vector<const sim_mob::RoadSegment*> > route_RoadSegments;
    //std::map<int, std::vector<const sim_mob::BusStopInfo*> > route_BusStopInfos;

#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil){}
    virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif
};

}

