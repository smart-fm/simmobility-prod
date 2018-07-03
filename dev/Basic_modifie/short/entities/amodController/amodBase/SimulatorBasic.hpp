/*
 * SimulatorBasic.hpp
 *
 *  Created on: Mar 27, 2015
 *      Author: haroldsoh
 */

#pragma once

#include "World.hpp"
#include "Simulator.hpp"
#include "Booking.hpp"
#include "Utility.hpp"
#include "KDTree.hpp"

#include <cmath>
#include <unordered_map>
#include <random>
#include <iostream>
#include <ctime>
#include <cstdio>

namespace sim_mob{

namespace amod {

class SimulatorBasic: public Simulator {
public:
    /**
     * constructor
     * initalize the simulator, optionally with a simulation resolution in seconds
     * @param resolution_ simulation resolution (default resolution is 0.1 seconds.)
     */
    SimulatorBasic(double resolution_ = 0.1);

    /**
     * Destructor
     */
	virtual ~SimulatorBasic();

    /**
     * init
     * initializes the Simulator with the world_state.
     * @param pointer to the amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode  init(amod::World *worldState);
    
    /**
     * update
     * updates the world_state
     * @param pointer to the amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode  update(amod::World *worldState);

    /// low level commands

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
                                             amod::VehicleStatus vehStartStatus = VehicleStatus::BUSY,
                                             amod::VehicleStatus vehEndStatus = VehicleStatus::FREE
                                             );
    
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
                                            amod::VehicleStatus endStatus = VehicleStatus::HIRED);
    
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
                                             amod::VehicleStatus endStatus = VehicleStatus::FREE);

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
                                             amod::CustomerStatus custEndStatus = CustomerStatus::FREE);

    /**
     * setCustomerStatus
     * sets a customer status
     * @param worldState pointer to the amod world
     * @param custId customer Id
     * @param status customer status
     */
    virtual void setCustomerStatus(amod::World *worldState, int custId, CustomerStatus status);


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
    virtual amod::ReturnCode serviceBooking(amod::World *worldState, const amod::Booking &booking);

    /// distance functions
    /**
     * getDrivingDistance
     * returns the driving distance for Position from to Position to.
     * This may not be the Euclidean distance on a road network
     * @param from source position
     * @param to destination position
     * @return driving distance b/w source and dest
     */
    virtual double getDrivingDistance(const amod::Position &from, const amod::Position &to);

    /**
     * getDrivingDistance
     * returns the driving distance for location from to location to.
     * This may not be the Euclidean distance on a road network
     * @param fromLocId source location id
     * @param toLocId destination location id
     * @return driving distance b/w source and dest
     */
    virtual double getDrivingDistance(int fromLocId, int toLocId);
    
    /**
     * getDistance
     * returns the Euclidean distance from Position from to Position to.
     * @param from source position
     * @param to destination position
     * @return euclidean distance b/w source and dest
     */
    virtual double getDistance(const amod::Position &from, const amod::Position &to);
    
    /// SimulatorBasic specific functions

    /// Sets the parameters of the distributions used in the basic simulator. All the
    /// distributions used are truncated normals with parameters mean, standard dev,
    /// minimum and maximum.
    /**
     * setVehicleSpeedParams
     * sets the vehicles speed distribution parameters in m/s
     * @param mean
     * @param sd - standard deviation
     * @param min
     * @param max
     */
    virtual void setVehicleSpeedParams(double mean, double sd, double min, double max);
    
    /**
     * setPickupDistributionParams
     * sets the pickup time distribution parameters in seconds
     * @param mean
     * @param sd - standard deviation
     * @param min
     * @param max
     */
    virtual void setPickupDistributionParams(double mean, double sd, double min, double max);
    
    /**
     * setDropoffDistributionParams
     * sets the dropoff time distribution parameters in seconds
     * @param mean
     * @param sd - standard deviation
     * @param min
     * @param max
     */
    virtual void setDropoffDistributionParams(double mean, double sd, double min, double max);

    /**
     * setTeleportDistributionParams
     * sets the teleportation time distribution parameters in seconds
     * @param mean
     * @param sd - standard deviation
     * @param min
     * @param max
     */
    virtual void setTeleportDistributionParams(double mean, double sd, double min, double max);
private:
    /// resolution of simulation in seconds
    double resolution;

    /// ideally, the true simulator will maintain it's own internal state
    amod::World state;

    /// KDTree to store locations
    kdt::KDTree<amod::Location> locTree;

    /// flag to verify if using location tree
    bool usingLocs;

    /// objects for random number generation
    /// Random number generator engine
    std::default_random_engine eng;

    /// Normal distribution
    std::normal_distribution<> normalDist;
    
    /// so that events have different ids
    long long eventId;
    
    /// structures to manage dispatches, pickups and dropoffs
    struct Dispatch {
        int bookingId;  ///0 if manual dispatch
        int vehId;
        int toLocId;
        int fromLocId;
    	Position from;
    	Position to;
        Position grad; ///normalized gradient
        Position curr;
        amod::VehicleStatus vehEndStatus;
    };
    
    struct Pickup {
        int bookingId;  ///0 if manual dispatch
        int vehId;
        int custId;
        int locId;
        double pickupTime;
        amod::VehicleStatus vehEndStatus;
    };
    
    struct Dropoff {
        int bookingId;  ///0 if manual dispatch
        int vehId;
        int custId;
        int locId;
        double dropoffTime;
        amod::VehicleStatus vehEndStatus;
    };
    
    struct Teleport {
        int custId;
        int locId;
        double teleportArrivalTime;
        amod::CustomerStatus custEndStatus;
    };

    struct TruncatedNormalParams {
        std::normal_distribution<>::param_type par;
        double min;
        double max;
    };

    /// Container to store the bookings
    std::unordered_map<int, Booking> bookings;

    /// Container to store the dispatches
    std::unordered_map<int, Dispatch> dispatches;

    /// Container to store the pickups
    std::multimap<double, Pickup> pickups;

    /// Container to store the dropoffs
    std::multimap<double, Dropoff> dropoffs;

    /// Container to store the teleports
    std::multimap<double, Teleport> teleports;
    
    /// parameters for pickup distribution simulation
    TruncatedNormalParams pickupParams;
    
    /// parameters for dropoff distribution simulation
    TruncatedNormalParams dropoffParams;

    /// parameters for vehicle speeds
    TruncatedNormalParams speedParams;

    /// parameters for teleportation time
    TruncatedNormalParams teleportParams;

    /// internal functions
    /**
     * simulateVehicles
     * @param worldState - Pointer to the amod world
     */
    virtual void simulateVehicles(amod::World *worldState);

    /**
     * simulateCustomers
     * @param worldState - Pointer to the amod world
     */
    virtual void simulateCustomers(amod::World *worldState);

    /**
     * simulatePickups
     * @param worldState - Pointer to the amod world
     */
    virtual void simulatePickups(amod::World *worldState);

    /**
     * simulateDropoffs
     * @param worldState  - Pointer to the amod world
     */
    virtual void simulateDropoffs(amod::World *worldState);

    /**
     * simulateTeleports
     * @param worldState - Pointer to the amod world
     */
    virtual void simulateTeleports(amod::World *worldState);
    
    
    /// internal helper functions that really execute the dispatch, pickup and dropoff functions
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
     * @param bookingId
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode dispatchVehicle(amod::World *worldState,
                                             int vehId,
                                             const amod::Position &to,
                                             amod::VehicleStatus vehStartStatus,
                                             amod::VehicleStatus vehEndStatus,
                                             int bookingId);
    
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
     * @param bookingId
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode pickupCustomer(amod::World *worldState,
                                            int vehId,
                                            int custId,
                                            amod::VehicleStatus startStatus,
                                            amod::VehicleStatus endStatus,
                                            int bookingId);  
    
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
     * @param bookingId
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode dropoffCustomer(amod::World *worldState,
                                             int vehId,
                                             int custId,
                                             amod::VehicleStatus startStatus,
                                             amod::VehicleStatus endStatus,
                                             int bookingId);
    
    /**
     * genRandTruncNormal
     * generates the random truncated normal
     * @param params
     * @return
     */
    virtual double genRandTruncNormal(TruncatedNormalParams &params);

    /**
     * hasArrived
     * checks if the vehicle specified in the Dispatch d has arrived
     * @param d dispatch
     * @return true if arrived
     */
    virtual bool hasArrived(const Dispatch &d);
    
    /**
     * eucDist
     * returns the euclidean distance between a and b
     * @param a
     * @param b
     * @return euclidean distance between a and b
     */
    double eucDist(const Position &a, const Position &b);
};

}

}
