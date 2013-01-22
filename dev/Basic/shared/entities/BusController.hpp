/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "entities/Agent.hpp"

#include "buffering/Shared.hpp"
#include "entities/UpdateParams.hpp"
#include "misc/BusTrip.hpp"
#include "misc/PublicTransit.hpp"
#include "vehicle/Bus.hpp"
#include "util/DynamicVector.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"

namespace sim_mob {

class Bus;


/*
 * BusController class.
 * \author Yao Jin
 */
class BusController : public sim_mob::Agent {
private:
	explicit BusController(int id=-1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered) : Agent(mtxStrat, id),
		frameNumberCheck(0), nextTimeTickToStage(0), tickStep(1), firstFrameTick(true), currLink(nullptr)
	{}

public:
	///Initialize a single BusController with the given start time and MutexStrategy.
	///TODO: Currently having more than 1 BusController on different Workers is dangerous.
	static void RegisterNewBusController(unsigned int startTime, const MutexStrategy& mtxStrat);

	///Returns true if we have at least one bus controller capable of dispatching buses.
	static bool HasBusControllers();

	///This is a hack for now; any function that uses this is doing something that I'm not 100% clear on. ~Seth
	static BusController* TEMP_Get_Bc_1();

	///Initialize all bus controller objects based on the parameters loaded from the database/XML.
	static void InitializeAllControllers(std::vector<sim_mob::Entity*>& agents_list, std::vector<sim_mob::PT_bus_dispatch_freq>& busdispatch_freq);

	///Place all BusController agents on to the all_agents list. This does *not* add them to Worker threads (since those likely haven't been created yet).
	static void DispatchAllControllers(std::vector<sim_mob::Entity*>& agents_list);

public:
	//May implement later
	virtual void load(const std::map<std::string, std::string>& configProps){}

	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	virtual Entity::UpdateStatus update(timeslice now);

	void receiveBusInformation(const std::string& busline_i, int trip_k = 0, int busstopSequence_j = 0, unsigned int ATijk = 0);
	unsigned int decisionCalculation(const std::string& busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk, int lastVisited_BusStopSeqNum);// return Departure MS from Aijk, DWijk etc
	//unsigned int sendBusInformation();// depend on the control strategy
	void addOrStashBuses(Agent* p, std::vector<Entity*>& active_agents);

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
	void assignBusTripChainWithPerson(std::vector<Entity*>& active_agents);

	///Load all bus items from the database.
	void setPTScheduleFromConfig(std::vector<sim_mob::PT_bus_dispatch_freq>& busdispatch_freq);

	//Functions required by Jenny's code.
	// TODO: These shouldn't have to be duplicated across all entity types.
	virtual Link* getCurrLink();
	virtual void setCurrLink(Link* link);

private:
	//Note: For now, we have to store pointers, since the all_agents array is cleared and deleted on exit.
	//      Otherwise, it will attempt to delete itself twice. ~Seth
	static std::vector<BusController*> all_busctrllers_;

	void dispatchFrameTick(timeslice now);
	void frame_init(timeslice now);
	void frame_tick_output(timeslice now);

	unsigned int scheduledDecision(const std::string& busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk);// scheduled-based control
	unsigned int headwayDecision(const std::string& busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk); // headway-based control
	unsigned int evenheadwayDecision(const std::string& busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk, int lastVisited_BusStopSeqNum); // evenheadway-based control
	unsigned int hybridDecision(const std::string& busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk); // hybrid-based control(evenheadway while restricting the maximum holding time)
	unsigned int dwellTimeCalculation(const std::string& busline_i, int trip_k, int busstopSequence_j); // dwell time calculation module

	uint32_t frameNumberCheck;// check some frame number to do control
	uint32_t nextTimeTickToStage;// next timeTick to be checked
	unsigned int tickStep;
	bool firstFrameTick;  ///Determines if frame_init() has been done.
	std::vector<Bus*> managedBuses;// Saved all virtual managedBuses
	StartTimePriorityQueue pending_buses; //Buses waiting to be added to the simulation, prioritized by start time.
	DPoint posBus;// The sent position of a given bus ,only for test

	PT_Schedule pt_schedule;
	//The current Link. Used by Jenny's code (except we don't currently use buses in the medium term)
    sim_mob::Link* currLink;

#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil){}
    virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif
};

}

