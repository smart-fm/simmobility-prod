//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>

#include "Driver.hpp"
#include "BusDriverFacets.hpp"
#include "entities/misc/BusTrip.hpp"
#include "entities/Person_ST.hpp"

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
	std::list<Person_ST *> passengerList;
	
	/**Id of the bus line driven by the driver*/
	std::string busLineId;
	
	/**Sequence number of the bus*/
	unsigned int sequenceNum;
	
	/**The boarding time at the current bus stop*/
	double currBoardingTime;
	
	/**The alighting time at the current bus stop*/
	double currAlightingTime;
	
	/**The current bus stop agent*/
	BusStopAgent *currBusStopAgent;
	
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
	 * Message handler to handle messages transfered from the parent agent.
	 * 
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message);
	
	/**
	 * Check whether the bus is full
	 * 
	 * @return true if bus is full, else false
	 */
	bool isBusFull();
	
	/**
	 * Allows a boarding passenger into the bus if there is enough space and sends it the boarding success message, 
	 * else sends out a denied boarding message to the person
	 * 
	 * @param passenger the person attempting to board the bus
	 */
	void tryBoardingPassenger(Person_ST *passenger);
	
	/**
	 * Allows alighting of passengers at the bus stop
	 * 
	 * @param passenger the person attempting to alight the bus
	 */
	void alightPassenger(Person_ST *passenger);

	double getPositionX() const;
	double getPositionY() const;
	
	const std::string& getBusLineId() const;
	void setBusLineId(const std::string &busLine);

	BusStopAgent* getCurrBusStopAgent() const;

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
