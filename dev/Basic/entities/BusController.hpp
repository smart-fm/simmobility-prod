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

namespace sim_mob
{
class Bus;

class BusController : public sim_mob::Agent
{
public:
//	static BusController & getInstance() {return sim_mob::BusController::instance_;}
//	static BusController * get_pInstance() {return &(sim_mob::BusController::instance_);}
	explicit BusController(const MutexStrategy& mtxStrat = sim_mob::ConfigParams::GetInstance().mutexStategy, int id=-1);
	~BusController();
	virtual Entity::UpdateStatus update(frame_t frameNumber);
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);
	void updateBusInformation(DPoint pt);
	void DispatchInit();// similar to AddandStash --> p.start = 0
	void DispatchFrameTick(frame_t frameTick);
	bool isTobeOutput() { return TobeOutput; }
	void setTobeOutput() { TobeOutput = true; }

	// Manage Buses
	void addBus(Bus* bus);
	void remBus(Bus* bus);
	///Retrieve a reference to the list of managedBuses.
	std::vector<Bus*>& getManagedBuses() { return managedBuses; }
	//const sim_mob::RoadNetwork& getNetwork() { return network; }

private:
	void frame_init(frame_t frameNumber);
	void frame_tick_output(frame_t frameNumber);
	//static BusController instance_;

	frame_t frameNumberCheck;// check some frame number to do control
	frame_t nextTimeTickToStage;// next timeTick to be checked
	unsigned int tickStep;
	bool firstFrameTick;  ///Determines if frame_init() has been done.
	bool TobeOutput;// Determines whether Xml has buscontroller thus to be updated in output file
	std::vector<Bus*> managedBuses;// Saved managedBuses
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
extern sim_mob::BusController busctrller;
