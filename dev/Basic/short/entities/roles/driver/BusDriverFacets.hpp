/*
 * BusDriverFacets.hpp
 *
 *  Created on: May 16th, 2013
 *      Author: Yao Jin
 */

#pragma once
#include "conf/settings/DisableMPI.h"
#include "entities/roles/RoleFacets.hpp"
#include "entities/vehicle/Bus.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "BusDriver.hpp"


namespace sim_mob {

class BusDriver;
//class Bus;

class BusDriverBehavior: public sim_mob::DriverBehavior {
public:
	explicit BusDriverBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~BusDriverBehavior();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);

	BusDriver* getParentBusDriver() const {
		return parentBusDriver;
	}

	void setParentBusDriver(BusDriver* parentBusDriver) {
		this->parentBusDriver = parentBusDriver;
	}

protected:
	BusDriver* parentBusDriver;
};

class BusDriverMovement: public sim_mob::DriverMovement {
public:
	explicit BusDriverMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~BusDriverMovement();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual void flowIntoNextLinkIfPossible(UpdateParams& p);

	BusDriver* getParentBusDriver() const {
		return parentBusDriver;
	}

	void setParentBusDriver(BusDriver* parentBusDriver) {
		this->parentBusDriver = parentBusDriver;
	}

	/// Return the distance (m) to the (next) bus stop.
	/// A negative return value indicates that there is no relevant bus stop nearby.
	double distanceToNextBusStop();

	/// get distance to bus stop of particular segment (meter)
	/// A negative return value indicates that there is no relevant bus stop nearby.
	double getDistanceToBusStopOfSegment(const RoadSegment* rs);
	bool isBusFarawayBusStop();
	bool isBusApproachingBusStop();
	bool isBusArriveBusStop();
	bool isBusLeavingBusStop();
	bool isBusGngtoBreakDown();
	double busAccelerating(DriverUpdateParams& p);
	//mutable double lastTickDistanceToBusStop;


	///here passenger initially chooses which bus lines to board upon reaching bus stop
	///and board the bus when it approaches based on this initial choice
	void BoardingPassengers_Choice(Bus* bus);
	///here passenger makes decision to board bus when bus reaches bus stop
	///if the bus goes to the destination passenger decides to board
	void BoardingPassengers_Normal(Bus* bus);
	// new boarding method
	// individual boarding and alighting(Yao Jin)
	void IndividualBoardingAlighting_New(Bus* bus);
	// determine boarding and alighting MS for possible boarding and alighting persons(Yao Jin)
	void DetermineBoardingAlightingMS(Bus* bus);
	// start boarding and alighting based on the boarding and alighting MS
	// change passenger amount on the Bus(Yao Jin)
	void StartBoardingAlighting(Bus* bus);
	// reset some boarding and alighting variables after leaving the BusStop(Yao Jin)
	void resetBoardingAlightingVariables();

    void AlightingPassengers(Bus* bus);

    ///dwell time calculation module
 	virtual double dwellTimeCalculation(int A,int B,int delta_bay,int delta_full,int Pfront,int no_of_passengers); // dwell time calculation module
 	std::vector<const sim_mob::BusStop*> findBusStopInPath(const std::vector<const sim_mob::RoadSegment*>& path) const;

// 	double getPositionX() const;
// 	double getPositionY() const;

	// get total waiting time at the BusStop
	double getWaitTime_BusStop() { return BUS_STOP_WAIT_TIME; }
	// set total waiting time at the BusStop
	void setWaitTime_BusStop(double time) { BUS_STOP_WAIT_TIME = time; }
	// initialize Bus Path by BusTrip information
	Vehicle* initializePath_bus(bool allocateVehicle);

	double lastTickDistanceToBusStop;
	bool demo_passenger_increase;
	double dwellTime_record;// set by BusDriver(temporary), only needed by BusDriver
	//double xpos_approachingbusstop,ypos_approachingbusstop;
	bool first_busstop;
	bool last_busstop;
	bool passengerCountOld_display_flag;
	size_t no_passengers_boarding;
	size_t no_passengers_alighting;

	// flag to indicate whether boarding and alighting is allowed, if it is false, boarding alighting frame is not determined(reset after BusDriver leaves the BusStop)
	bool allow_boarding_alighting_flag;
	// a tempoary boarding queue, will be cleared after BusDriver leaves the BusStop
	std::vector<sim_mob::Person*> virtualBoarding_Persons;
	// record the BoardingNum_Pos map based on the boarding queue in the BusStopAgent, cleared after BusDriver leaves the BusStop
	std::map<int, int> BoardingNum_Pos;
	// record the AlightingNum_Pos map based on the passenger queue in the Bus, cleared after BusDriver leaves the BusStop
	std::map<int, int> AlightingNum_Pos;
	// boarding_MSs for possible boarding persons, cleared after leaving the BusStop
	std::vector<uint32_t> boarding_MSs;
	// alighting_MSs for possible alighting persons, cleared after leaving the BusStop
	std::vector<uint32_t> alighting_MSs;
	// the first boarding and alighting MS where bus will start boarding and alighting, reset after leaving the BusStop
	uint32_t first_boarding_alighting_ms;
	// the last boarding and alighting MS and bus will leaves the BusStop, reset after leaving the BusStop
	uint32_t last_boarding_alighting_ms;
	// temporary boardingMS offset for boarding queue erase purpose, reset after leaving the BusStop
	int boardingMS_offset;
	// temporary alightingMS offset for passenger queue erase purpose, reset after leaving the BusStop
	int alightingMS_offset;
	// holdingtime SECS
	double BUS_STOP_HOLDING_TIME_SEC;
	// dwelltime(boarding and alighting time SECS)
	double BUS_STOP_WAIT_BOARDING_ALIGHTING_SEC;

protected:
	//Override the following behavior
	virtual double linkDriving(DriverUpdateParams& p);

protected:
	BusDriver* parentBusDriver;

//Basic data
private:
	// total busStop list
	std::vector<const BusStop*> busStops;
	// waiting MS adding at the BusStop
	double waitAtStopMS;
	// total waiting time (can be holding time or dwelltime)
	double BUS_STOP_WAIT_TIME;
};
}
