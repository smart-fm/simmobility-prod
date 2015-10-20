//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Driver.hpp"
#include "BusDriverFacets.hpp"
#include "buffering/Shared.hpp"
#include "entities/misc/BusTrip.hpp"
#include "entities/Person_MT.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/DriverRequestParams.hpp"

namespace sim_mob {

namespace medium
{
class BusDriverBehavior;
class BusDriverMovement;
class BusStopAgent;
class Passenger;

class BusDriver : public Driver {
public:
	BusDriver(Person_MT* parent, const MutexStrategy& mtxStrat,
			BusDriverBehavior* behavior = nullptr,
			BusDriverMovement* movement = nullptr,
			std::string roleName = std::string(),
			Role<Person_MT>::Type roleType = Role<Person_MT>::RL_BUSDRIVER);
	virtual ~BusDriver();

	virtual Role<Person_MT>* clone(Person_MT* parent) const;

	//Virtual overrides
	virtual void make_frame_tick_params(timeslice now);
	virtual std::vector<BufferedBase*> getSubscriptionParams();
	virtual DriverRequestParams getDriverRequestParams();

	/**
	 * gets stops list from bus route
	 * @return constant pointer to bus stops list
	 */
	const std::vector<const BusStop*>* getBusStopsVector() const;

	/**
	 * insert a new passenger to bus driver when a passenger
	 * decide to board on this bus
	 *
	 * @param passenger is the pointer to the person who will board on this bus
	 *
	 * @return boolean value, if boarding success, the value is true;
	 * otherwise this value is false.
	 */
	bool addPassenger(Passenger* passenger);

	/**
	 * store the arrival time at bus stop
	 *
	 * @param current is current time represented in string
	 * @param stop is which currently bus driver arrive at
	 *
	 */
	void storeArrivalTime(const std::string& current, const std::string& waitTime, const BusStop* stop);

	/**
	 * change whether bus is full already
	 *  @return boolean value, if bus is full, return true, otherwise false
	 */
	bool checkIsFull();

	/**
	 * predict arrival at next bus stop in next frame tick
	 * @param preArrivalTime is predicted arrival time at next bus stop
	 * @param bus stop agent is the agent which wrap next bus stop
	 */
	void predictArrivalAtBusStop(double preArrivalTime, BusStopAgent* busStopAgent);

	/**
	 * get current waiting time at bus stop
	 */
	double getWaitingTimeAtBusStop() {
		return waitingTimeAtbusStop;
	}

	const std::string getBusLineID() const;

	void calcTravelTime();

	int busSequenceNumber;

private:
	/**passengers list*/
	std::list<Passenger*> passengerList;
	/**last visited bus stop*/
	Shared<const BusStop*> visitedBusStop;
	/**last visited bus stop sequence number*/
	Shared<int> visitedBusStopSequenceNo;
	/**real arrival time set by bus driver*/
	Shared<double> arrivalTime;
	/**current bus stop real times including departure and arrival time*/
	Shared<BusStopRealTimes>* busStopRealTimes;
	/** dwell time set by bus driver*/
	Shared<double> dwellTime;
	/**holding time set by bus controller*/
	Shared<double> holdingTime;
	/**get bus line information*/
	Shared<std::string> visitedBusLine;
	/**sequence number for current bus trip*/
	Shared<int> visitedBusTripSequenceNo;
	/**request mode for holding strategy*/
	Shared<int> requestMode;
	/**final waiting time at bus stop*/
	double waitingTimeAtbusStop;

	/**
	 * alight passengers when those want to alight at next bus stop
	 * @param bus stop agent is the agent which wrap bus stop and waiting people
	 * @return the number of alighting people
	 */
	unsigned int alightPassenger(BusStopAgent* busStopAgent);

	/**
	 * triggers boarding and alighting at a bus stop
	 * @param busStopAgent agent managing the stop which is currently served
	 * @param current is current time represented in string
	 */
	void openBusDoors(const std::string& current, BusStopAgent* busStopAgent);

	/**
	 * triggers bus departure from a bus stop
	 * @param busStopAgent agent managing the stop which is currently served
	 */
	void closeBusDoors(BusStopAgent* busStopAgent);

	friend class BusDriverBehavior;
	friend class BusDriverMovement;


};

}
}


