//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PassengerFacets.hpp
 *
 *  Created on: June 17th, 2013
 *      Author: Yao Jin
 */

#pragma once
#include "conf/settings/DisableMPI.h"
#include "entities/roles/RoleFacets.hpp"
#include "Passenger.hpp"

namespace sim_mob {

class Passenger;
class BusDriver;

class PassengerBehavior: public sim_mob::BehaviorFacet {
public:
	explicit PassengerBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~PassengerBehavior();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	Passenger* getParentPassenger() const {
		return parentPassenger;
	}

	void setParentPassenger(Passenger* parentPassenger) {
		this->parentPassenger = parentPassenger;
	}

private:
	Passenger* parentPassenger;

};

class PassengerMovement: public sim_mob::MovementFacet {
public:
	explicit PassengerMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~PassengerMovement();

	//Virtual overrides
	void setParentBufferedData();
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	bool isAtBusStop();
	bool isBusBoarded();
	bool isDestBusStopReached();
	Point2D getXYPosition();
	Point2D getDestPosition();
	const BusStop* getOriginBusStop() { return OriginBusStop; }
	const BusStop* getDestBusStop() { return DestBusStop; }

	///NOTE: These boarding/alighting functions are called from BusDriver and used to transfer data.

	///passenger boards the approaching bus if it goes to the destination
	///.Decision to board is made when the bus approaches the busstop.So the first
	///bus which would take to the destination would be boarded
	bool PassengerBoardBus_Normal(BusDriver* busdriver,std::vector<const BusStop*> busStops);

	bool PassengerAlightBus(Driver* busdriver);

	///passenger has initially chosen which bus lines to board and passenger boards
	///the bus based on this pre-decision.Passenger makes the decision to board a bussline
	///based on the path of the bus if the bus goes to the destination and chooses the busline based on shortest distance
	bool PassengerBoardBus_Choice(Driver* busdriver);

	///to find waiting time for passengers who have boarded bus,time difference between
	/// time of reaching busstop and time bus reaches busstop
	void findWaitingTime(Bus* bus);

	///finds the nearest busstop for the given node,As passenger origin and destination is given in terms of nodes
	BusStop* setBusStopXY(const Node* node);

	///finds which bus lines the passenger should take after reaching the busstop
	///based on bussline info at the busstop
	void FindBusLines();

	std::vector<Busline*> ReturnBusLines();

	//bool isOnCrossing() const;
	Passenger* getParentPassenger() const {
		return parentPassenger;
	}
	void setParentPassenger(Passenger* parentPassenger) {
		this->parentPassenger = parentPassenger;
	}

public:
	uint32_t alighting_MS;// to record the alighting_MS for each individual person

private:
	Passenger* parentPassenger;
	BusStop* OriginBusStop;///busstop passenger is starting the trip from
    BusStop* DestBusStop;///busstop passenger is ending the trip

	std::vector<Busline*> BuslinesToTake;///buslines passenger can take;decided by passenger upon reaching busstop
	double WaitingTime;
	double TimeOfReachingBusStop;
	///For display purposes: offset this Passenger by a given +x, +y
	Point2D DisplayOffset;

	///for display purpose of alighting passengers
	int displayX;
	int displayY;

	///to display alighted passenger for certain no of frame ticks before removal
	int skip;
};
}
