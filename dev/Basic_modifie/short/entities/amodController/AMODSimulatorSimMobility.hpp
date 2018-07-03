/*
 * AMODSimulatorSimMobility.h
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#pragma once

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <iostream>
#include <utility>
#include <array>
#include <mutex>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <string>

#include <boost/regex.hpp>

#include "amodBase/KDTree.hpp"
#include "amodBase/Types.hpp"
#include "amodBase/Entity.hpp"
#include "amodBase/Simulator.hpp"

#include "entities/Agent.hpp"
#include "entities/Person_ST.hpp"
#include "event/args/EventArgs.hpp"
#include "event/EventPublisher.hpp"

#include "path/PathSetManager.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

#include "entities/misc/TripChain.hpp"
#include "entities/roles/Role.hpp"
#include "entities/vehicle/VehicleBase.hpp"

#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/Node.hpp"

#include "logging/Log.hpp"
#include "metrics/Frame.hpp"
#include "workers/Worker.hpp"

namespace sim_mob {
namespace amod {


class AMODSimulatorSimMobility: public amod::Simulator {
public:
    /**
     * Constructor
     * @param parentController
     */
    AMODSimulatorSimMobility(sim_mob::Agent *parentController);

    /**
     * Destructor
     */
	virtual ~AMODSimulatorSimMobility();

    /**
     * init
     * initializes the Simulator with the world_state.
     * @param pointer to the amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode init(amod::World *worldState);

    /**
     * update
     * updates the world_state
     * @param pointer to the amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode update(amod::World *worldState);

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
                                             amod::VehicleStatus vehStartStatus = amod::VehicleStatus::BUSY,
                                             amod::VehicleStatus vehEndStatus = amod::VehicleStatus::FREE
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
                                            amod::VehicleStatus startStatus = amod::VehicleStatus::PICKING_UP,
                                            amod::VehicleStatus endStatus = amod::VehicleStatus::HIRED);

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
											 amod::VehicleStatus status = amod::VehicleStatus::DROPPING_OFF,
                                             amod::VehicleStatus endStatus = amod::VehicleStatus::FREE);

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
                                             amod::CustomerStatus custStartStatus = amod::CustomerStatus::TELEPORTING,
                                             amod::CustomerStatus custEndStatus = amod::CustomerStatus::FREE);

    /**
     * setCustomerStatus
     * sets a customer status
     * @param worldState pointer to the amod world
     * @param custId customer Id
     * @param status customer status
     */
    virtual void setCustomerStatus(amod::World *worldState, int custId, amod::CustomerStatus status);


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

    //virtual void preComputeDrivingDistances(); // precomputes driving distance (can take a very long time with a large number of nodes)

    /**
     * loadPreComputedDistances
     * load pre-computed distances from the specified file
     * @param filename precomputed distance file name
     * @update flag to specify whether update the distances
     */
    virtual void loadPreComputedDistances(const std::string& filename, bool update=false);

    /**
     * getDistance
     * returns the Euclidean distance from Position from to Position to.
     * @param from source position
     * @param to destination position
     * @return euclidean distance b/w source and dest
     */
    virtual double getDistance(const amod::Position &from, const amod::Position &to);

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

    /// simmobility specific functions
    /**
     * setController
     * set parent controller
     * @param parentController_ parant controller (SimMobility Agent)
     */
    virtual void setController(sim_mob::Agent *parentController_);

    /**
     * setCurrentTimeMs
     * set current time in ms
     * @param currTimeMS - current time in MS
     */
    virtual void setCurrentTimeMs(int currTimeMS);

    /**
     * setCurrentTime
     * set current time in seconds
     * @param currTime - current time in secs
     */
    virtual void setCurrentTime(double currTime);

    /**
     * getCurrentTime
     * get current time in seconds
     * @return current time in secs
     */
    virtual double getCurrentTime();

    /**
     * getPositionInSimmobility
     * get position of a vehicle in simmobility
     * @param worldState - Pointer to the amod world
     * @param vehId - Vehicle id
     * @return Position of vehicle in simmobility
     */
    virtual amod::Position getPositionInSimmobility(amod::World *worldState, int vehId);

    /**
     * setArrival
     * callback function for simmobility to call
     * @param amodId - amod controller id
     * @param currTimeSecs - current time in seconds
     */
    virtual void setArrival(int amodId, int currTimeSecs);
    
    /**
     * setSaveNodesFilename
     * @param filename save nodes file name
     */
    virtual void setSaveNodesFilename(const std::string& filename) {
        saveNodesFileName = filename;
    }

    void initWaitingTimeFile(const std::string& fileName);

    bool logEnabled = false;

private:
    /// simmobility current time (seconds)
    double currentTime;

    /// random number generation
    /// Random number engine
    std::default_random_engine eng;

    /// Normal distribution
    std::normal_distribution<> normalDist;

    /// each events shall have different ids
    long long eventId;

    /// each trips shall have different ids
    long long tripId;

    /// pointer to controller
    sim_mob::Agent *parentController;

    /// map to cache shortest path distances
    std::unordered_map< int, std::unordered_map< int, double > > drivingDistances;

    /// distance file stream
    std::fstream distFile;

    /// output file stream for logging
    std::ofstream waitingTimeFile;

    /// flag to determine whether the distance file should be updated or not
    bool updDistFile;
    
	// objects in the simulated world (typically vehicles)
	//std::unordered_map<int, amod::Vehicle> vehicles_;
	//std::unordered_map<int, amod::Customer> customers_;

    std::string saveNodesFileName;

    /// Container to map amod vehicle id and its respective agent in simmobility
    std::unordered_map<int, sim_mob::Person_ST *> vehIdToPersonMap;

    /// store all nodes in a tree for fast lookups
    kdt::KDTree<amod::Location> locTree;

    /// maps from AIMSUM ids to node and segment objects
	// std::unordered_map<int,const sim_mob::RoadSegment*> seg_pool_; // store all segs ,key= aimsun id ,value = seg
    /// store all nodes ,key= aimsun id ,value = node
    std::map<unsigned int,sim_mob::Node*> nodePool;

    /// street directory
    sim_mob::StreetDirectory* stDir;

    /// arrivals vector to store arrivals and time of arrival
    std::mutex arrivalsMtx;
    std::vector<std::pair<int, double>> arrivals;
    
    /// error message
    std::string baseErrMsg;

    /// structures to manage dispatches, pickups and dropoffs
	struct Dispatch {
        int bookingId; ///0 if manual dispatch
        int vehId;
        int toLocId;
        int fromLocId;
		amod::Position from;
		amod::Position to;
		amod::Position curr;
        amod::VehicleStatus vehEndStatus;
        bool arrived;
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

    struct TruncatedNormalParams {
        std::normal_distribution<>::param_type par;
        double min;
        double max;
    };

    struct Teleport {
        int custId;
        int locId;
        double teleportArrivalTime;
        amod::CustomerStatus custEndStatus;
    };

    /// Container to store the bookings
    std::unordered_map<int, amod::Booking> bookings;

    ///dispatches stored by vehicle id
    std::unordered_map<int, Dispatch> dispatches;

    /// Container to store pickups
    std::multimap<double, Pickup> pickups;

    /// Container to store dropoffs
    std::multimap<double, Dropoff> dropoffs;

    /// Container to store teleports
    std::multimap<double, Teleport> teleports;

    /// parameters for pickup distribution simulation
    TruncatedNormalParams pickupParams;

    /// parameters for dropoff distribution simulation
    TruncatedNormalParams dropoffParams;

    /// parameters for vehicle speeds
    TruncatedNormalParams speedParams;

    /// parameters for teleportation time
    TruncatedNormalParams teleportParams;

    /// private internal functions
    /// functions that simulate the vehicles, customers, pickups and dropoffs
    /// at each time step
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
    /// helper functions
    /**
     * getNodePosition
     * Retrives the position of a node given its id
     * @param nodeId
     * @return position of node
     */
    virtual amod::Position getNodePosition(int nodeId);

    /**
     * findClosestNode
     * finds closest node for a given position
     * @param to - position for which closest node is found
     * @return - closest node id
     */
	virtual int findClosestNode(const amod::Position &to);

    /**
     * genRandTruncNormal
     * generates the random truncated normal
     * @param params
     * @return
     */
    virtual double genRandTruncNormal(TruncatedNormalParams &params);

    /// for simmobility debugging
    //std::set<int> destroyed_veh_ids_;
    /**
     * checkVehicleAlive
     * check whether the vehicle has arrived
     * @param vehId - vehicle id
     * @return true if vehicle has arrived, else false
     */
    bool checkVehicleAlive(int vehId);
};

}
}
