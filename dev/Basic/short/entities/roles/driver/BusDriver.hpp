//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>

#include "Driver.hpp"
#include "BusDriverFacets.hpp"
#include "entities/misc/BusTrip.hpp"
#include "entities/Person_ST.hpp"
#include "entities/vehicle/BusRoute.hpp"

namespace sim_mob
{

class DriverUpdateParams;
class PackageUtils;
class UnPackageUtils;
class BusStop;
class BusStopAgent;
class Person_ST;
class Passenger;
class BusDriverBehavior;
class BusDriverMovement;

class BusDriver : public Driver
{
private:
	/**Passengers in the bus*/
	std::list<Passenger *> passengerList;
	
	/**Id of the bus line driven by the driver*/
	std::string busLineId;
	
	/**Sequence number of the bus*/
	unsigned int sequenceNum;
	
public:
	BusDriver(Person_ST *parent, MutexStrategy mtxStrat, BusDriverBehavior *behavior = nullptr, BusDriverMovement *movement = nullptr,
			Role<Person_ST>::Type roleType_ = Role<Person_ST>::RL_BUSDRIVER);

	/**
	 * Creates and initialises the movement and behaviour objects required for the BusDriver role,
	 * assigns them to a new driver and returns a pointer to the driver.
     *
	 * @param parent the person who will be taking up the requested role
     *
	 * @return the created role
     */
	virtual Role<Person_ST>* clone(Person_ST *parent) const;

	/**
	 * Creates a vector of the subscription parameters and returns it
	 *
     * @return vector of the subscription parameters
     */
	virtual std::vector<BufferedBase *> getSubscriptionParams();

	/**
	 * Creates the Driver request parameters
	 * 
	 * @return the driver request parameters
	 */
	virtual DriverRequestParams getDriverRequestParams();
	
	/**
	 * Check whether the bus is full
	 * 
	 * @return true if bus is full, else false
	 */
	bool isBusFull();
	
	/**
	 * Adds passenger to the bus
	 * 
	 * @param passenger passenger to be added to the bus
	 */
	void addPassenger(Passenger *passenger);
	
	/**
	 * Allows alighting of passengers at the bus stop
	 * 
	 * @param stopAgent the bus stop agent to which the alighting passengers will be transferred
	 * 
	 * @return total time required for alighting 
	 */
	double alightPassengers(BusStopAgent *stopAgent);

	double getPositionX() const;
	double getPositionY() const;
	
	const std::string& getBusLineId() const;
	void setBusLineId(const std::string &busLine);

	friend class BusDriverBehavior;
	friend class BusDriverMovement;

#ifndef SIMMOB_DISABLE_MPI
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);
	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif

};
}
