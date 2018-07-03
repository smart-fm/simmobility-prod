/*
 * Vehicle.h
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#pragma once

#include "Types.hpp"
#include "Entity.hpp"

#include <list>

namespace sim_mob {
namespace amod {

class Vehicle : public Entity {
public:
    /**
     * Vehicle status
     */
    enum Status {
        FREE,
	    BUSY,
	    HIRED,
	    MOVING_TO_PICKUP,
	    MOVING_TO_DROPOFF,
	    PICKING_UP,
	    DROPPING_OFF,
	    PARKED,
	    MOVING_TO_REBALANCE,
	    UNKNOWN
	};

    /**
     * Constructor
     * @param id - vehicle id
     */
    Vehicle(int id = 0);

    /**
     * Constructor
     * @param id - vehicle id
     * @param name - vehicle name
     * @param pos - position
     * @param capacity - capacity of the vehicle
     * @param status - vehicle status
     */
    Vehicle(int id, const std::string& name, Position pos, int capacity, Vehicle::Status status);

    /**
     * Destructor
     */
	virtual ~Vehicle();

    /**
     * retrieves current vehicle status
     * @return vehicle status
     */
	virtual Vehicle::Status getStatus() const;

    /**
     * sets the vehicle status
     * @param s vehicle status
     */
	virtual void setStatus(Vehicle::Status s);

    /**
     * get the speed of the vehicle
     * @return speed
     */
	virtual double getSpeed() const;

    /**
     * set the speed of the vehicle
     * @param speed
     */
	virtual void setSpeed(double speed);

    /**
     * set customer id who is using the vehicle
     * @param custId_
     */
    virtual void setCustomerId(int custId_);

    /**
     * retrieves the customer id who is using the vehicle
     * @return customer id
     */
    virtual int getCustomerId() const;

    /**
     * clears the customer id
     */
	virtual void clearCustomerId();

    /**
     * sets the capacity of the vehicle
     * @param capacity
     */
	virtual void setCapacity(int capacity);

    /**
     * retrieves the capacity of the vehicle
     * @return capacity of the vehicle
     */
	virtual int getCapacity() const;

    /**
     * setWaypoints
     * @param waypoints
     */
    virtual void setWaypoints(std::list<Position> &waypoints);

    /**
     * getWaypoints
     * @param waypoints
     */
    virtual void getWaypoints(std::list<Position> *waypoints);

    /**
     * retrieves the location id
     * @return location id
     */
    virtual int getLocationId(); 

    /**
     * sets the location id
     * @param locId
     */
    virtual void setLocationId(int locId);
    
private:
    /// Vehicle status
    Vehicle::Status status;

    /// Vehicle capacity
    int capacity;

    /// Speed of the vehicle
    double speed;

    /// Serving customer id
    int custId;

    /// Way points
    std::list<Position> wayPoints;

    /// Location id
    int locId;
    
    // FOR DEBUGGING
    // TO REMOVE
public:
    /// To check whether the vehicle struck for a long time
    int stationaryCount;
};

typedef Vehicle::Status VehicleStatus;


}
}
