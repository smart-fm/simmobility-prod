/*
 * Simulator.h
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#pragma once

#include "Types.hpp"
#include "Entity.hpp"
#include "Vehicle.hpp"
#include "World.hpp"
#include "Booking.hpp"
#include <vector>

namespace sim_mob{

namespace amod {

class Simulator {
public:
    /**
     * Constructor
     */
    Simulator() : verbose(false) {}

    /**
     * Destructor
     */
    virtual ~Simulator() {}

    /**
     * init
     * initializes the Simulator with the world_state.
     * @param pointer to the amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode  init(amod::World *worldState) = 0;
    
    /**
     * update
     * updates the world_state
     * @param pointer to the amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode  update(amod::World *worldState) = 0;
    
    // low level commands
    
    /**
     * dispatchVehicle
     * dispatches vehicle with id veh_id to Position to. If the call is successful,
     * the vehicle status is status is set to start_status. When the vehicle arrives,
     * an event is triggered and the vehicle's status is set to end_status.
     * @param worldState pointer to the amod world
     * @param vehId vehicle Id
     * @param to destination position
     * @param vehStartStatus default status is busy
     * @param vehEndStatus default status is free
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode dispatchVehicle(amod::World *worldState,
					     int vehId,
                                             const amod::Position &to,
					     amod::VehicleStatus startStatus = VehicleStatus::BUSY,
					     amod::VehicleStatus endStatus = VehicleStatus::FREE) = 0;
    
    /**
     * pickupCustomer
     * picks up customer with id cust_id using vehicle with id veh_id. If the call is successful,
     * the vehicle status is status is set to start_status. After the customer is picked up,
     * an event is triggered and the vehicle's status is set to end_status.
     * @param worldState pointer to the amod world
     * @param vehId vehicle Id
     * @param custId customer Id
     * @param vehStartStatus default status is picking up
     * @param vehEndStatus default status is hired
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode pickupCustomer(amod::World *worldState,
					    int vehId, int custId,
					    amod::VehicleStatus startStatus = VehicleStatus::PICKING_UP,
					    amod::VehicleStatus endStatus = VehicleStatus::HIRED) = 0;
    
    /**
     * dropoffCustomer
     * drops off customer with id cust_id using vehicle with id veh_id. If the call is successful,
     * the vehicle status is status is set to start_status. After the customer is dropped off,
     * an event is triggered and the vehicle's status is set to end_status.
     * @param worldState pointer to the amod world
     * @param vehId vehicle Id
     * @param custId customer Id
     * @param vehStartStatus default status is dropping off
     * @param vehEndStatus default status is free
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode dropoffCustomer(amod::World *worldState,
					     int vehId, int custId,
                                             amod::VehicleStatus status = VehicleStatus::DROPPING_OFF,
					     amod::VehicleStatus endStatus = VehicleStatus::FREE) = 0;
    
    
    /**
     * teleportCustomer
     * teleports the customer with cust_id to location closest to Position to. This simulates
     * transport via train or some other mode that doesn't contribute to road congestion
     * When the custer arrives,
     * an event is triggered and the vehicle's status is set to end_status.
     * @param worldState pointer to the amod world
     * @param vehId vehicle Id
     * @param to destination position
     * @param vehStartStatus default status is teleporting
     * @param vehEndStatus default status is free
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode teleportCustomer(amod::World *worldState,
					     int custId,
					     const amod::Position &to,
					     amod::CustomerStatus custStartStatus = CustomerStatus::TELEPORTING,
					     amod::CustomerStatus custEndStatus = CustomerStatus::FREE) = 0;



    /**
     * setCustomerStatus
     * sets a customer status
     * @param worldState pointer to the amod world
     * @param custId customer Id
     * @param status customer status
     */
    virtual void setCustomerStatus(amod::World *worldState, int custId, CustomerStatus status) = 0;


    /// Medium level commands, i.e., makes basic tasks easier to do with default events
    /// automatically triggered.
    /**
     * serviceBooking
     * services the amod::Booking booking.
     * For teleportation, this simply teleports the customer to the destination.
     * For amod travel, this automatically simulates servicing a booking call
     * from dispatch to dropoff. Specifically:
     * The vehicle specified by booking.veh_id is dispatched from it's position to the position of
     * booking.cust_id (with status MOVING_TO_PICKUP). Upon arrival, an event is triggered.
     * The vehicle then waits to picks up the customer with status PICKING_UP. Upon pickup, an
     * event is triggered and the vehicle is then dispatched to the position booking.destination
     * with status MOVING_TO_DROPOFF. Upon arrival at the destination, an arrival event is triggered
     * and the vehicle begins to drop off the customer, with status DROPPING_OFF. When the passenger
     * is dropped off, a dropped off event is triggered and the vehicle is set to FREE.
     * @param worldState pointer to the amod world
     * @param booking
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode serviceBooking(amod::World *worldState, const amod::Booking &booking) = 0;
    
    /// distance functions
    /**
     * getDrivingDistance
     * returns the driving distance for Position from to Position to.
     * This may not be the Euclidean distance on a road network
     * @param from source position
     * @param to destination position
     * @return driving distance b/w source and dest
     */
    virtual double getDrivingDistance(const amod::Position &from, const amod::Position &to) = 0;

    /**
     * getDrivingDistance
     * returns the driving distance for location from to location to.
     * This may not be the Euclidean distance on a road network
     * @param fromLocId source location id
     * @param toLocId destination location id
     * @return driving distance b/w source and dest
     */
    virtual double getDrivingDistance(int fromLocId, int toLocId) = 0;
    
    /**
     * getDistance
     * returns the Euclidean distance from Position from to Position to.
     * @param from source position
     * @param to destination position
     * @return euclidean distance b/w source and dest
     */
    virtual double getDistance(const amod::Position &from, const amod::Position &to) = 0;

    /**
     * setVerbose
     * @param v
     */
    virtual void setVerbose(bool v) {
	verbose = v;
    }
    
    /**
     * getVerbose
     * @return true if verbose is enabled
     */
    virtual bool getVerbose() {
	return verbose;
    }
    
protected:
    /// Flag to identify whether to print messages during simulation
    bool verbose;
    
};

}

}
