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

    void AlightingPassengers(Bus* bus);

    ///dwell time calculation module
 	double dwellTimeCalculation(int A,int B,int delta_bay,int delta_full,int Pfront,int no_of_passengers); // dwell time calculation module
 	std::vector<const sim_mob::BusStop*> findBusStopInPath(const std::vector<const sim_mob::RoadSegment*>& path) const;

// 	double getPositionX() const;
// 	double getPositionY() const;

 	double getWaitTime_BusStop() { return BUS_STOP_WAIT_PASSENGER_TIME_SEC; }
 	void setWaitTime_BusStop(double time) { BUS_STOP_WAIT_PASSENGER_TIME_SEC = time; }
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

protected:
	//Override the following behavior
	virtual double linkDriving(DriverUpdateParams& p);

protected:
	BusDriver* parentBusDriver;

//Basic data
private:
	std::vector<const BusStop*> busStops;
	double waitAtStopMS;
	double BUS_STOP_WAIT_PASSENGER_TIME_SEC;
};
}
