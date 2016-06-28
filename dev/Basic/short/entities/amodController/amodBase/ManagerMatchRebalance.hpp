//
//  ManagerMatchRebalance.h
//  AMODBase
//
//  Created by Harold Soh on 29/3/15.
//  Copyright (c) 2015 Harold Soh. All rights reserved.
//

#pragma once

#include "Types.hpp"
#include "Manager.hpp"
#include "Booking.hpp"
#include "World.hpp"
#include "Event.hpp"
#include "KDTree.hpp"
#include "SimpleDemandEstimator.hpp"

#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cstdlib>

#include "glpk.h"

namespace sim_mob{

namespace amod {
class ManagerMatchRebalance : public Manager {
public:

    enum MatchMethod { ASSIGNMENT, GREEDY};
    enum BOOKING_DISCARD_REASONS { CUSTOMER_NOT_AT_LOCATION,
        CUSTOMER_NOT_FREE,
        SERVICE_BOOKING_FAILURE,
        NO_SUITABLE_PATH,
    };

    /**
     * Constructor
     */
    ManagerMatchRebalance();

    /**
     * Destructor
     */
    virtual ~ManagerMatchRebalance();

    /**
     * init
     * initializes the manager with the World world_state
     * @param worldState Pointer to the amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode init(World *worldState);

    /**
     * update
     * updates the manager with the World world_state. This manager is a simple
     * demonstration of the manager and is a simple queue (FIFO) manager which dispatches
     * the closest FREE or PARKED vehicle. Bookings are responded to by booking time.
     * If there are no available vehicles, the booking remains in the booking queue.
     * @param worldState Pointer to the amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode update(World *worldState);

    /**
     * loadBookings
     * loads bookings that the manager should respond to.
     * @param bookings list of bookings
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode loadBookings(const std::vector<Booking> &bookings);

    /**
     * loadBookingsFromFile
     * loads bookings from a file specified by filename that the manager should respond to.
     * @param filename Bookings file name
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode loadBookingsFromFile(const std::string& filename);

    /**
     * setMatchingMethod
     * sets the matching method to use
     * either ManagerMatchRebalance::ASSIGNMENT or ManagerMatchRebalance::GREEDY
     * inline function
     * @param m Match method (Greedy/Assignment)
     */
    virtual void setMatchMethod(ManagerMatchRebalance::MatchMethod m) {
        matchMethod = m;
    }

    /**
     * setCostFactors
     * sets the multiplicative factors for the individual matching cost components. Currently
     * there is the distance_cost_factor and the waiting_time_cost_factor
     * both factors default to 1.0
     * @param distCostFactor
     * @param waitingTimeCostFactor
     */
    virtual void setCostFactors(double distCostFactor, double waitingTimeCostFactor);

    /**
     * setMatchingInterval
     * set the matching interval
     * @param matchingInterval_ the default matching interval is 60
     */
    virtual void setMatchingInterval(double matchingInterval_);

    /**
     * getMatchingInterval
     * get the matching interval
     * @return matching interval
     */
    virtual double getMatchingInterval() const;

    /**
     * setRebalancingInterval
     * set rebalancing interval
     * @param rebalancingInterval_ the default rebalancing interval is 300 (every 5 minutes)
     */
    virtual void setRebalancingInterval(double rebalancingInterval_);

    /**
     * getRebalancingInterval
     * get rebalancing interval
     * @return rebalancing interval
     */
    virtual double getRebalancingInterval() const;

    /**
     * loadStations
     * loads the stations which are used to house vehicles
     * @params stations
     * @params worldState amod world
     */
    virtual void loadStations(std::vector<amod::Location> &stations, const amod::World &worldState);

    /**
     * setDemandEstimator
     * set the demand estimator
     * @param sde demand estimator to be used
     */
    virtual void setDemandEstimator(amod::DemandEstimator *sde);

    /**
     * useCurrentQueueForEstimation
     * @param useQueue
     */
    virtual void useCurrentQueueForEstimation(bool useQueue=true) {
        useCurrentQueue = useQueue;
    }

    /**
     * isUseCurrentQueueForEstimation
     * @return true if current queue can be used for estimations
     */
    virtual bool isUseCurrentQueueForEstimation() {
        return useCurrentQueue;
    }

private:
    /// Container to store the bookings
    std::multimap<double, Booking> bookings;

    /// Iterator to the bookings container
    std::multimap<double, Booking>::iterator bookingsItr;

    /// Booking file
    std::ifstream bookingFile;

    /// Flag to verify whether to use bookings file
    bool useBookingsFile;

    /// Last booking read
    Booking lastBookingRead;

    /// Event id
    int eventId;

    /// output file stream for logging
    std::ofstream outFile;

    /// Flag to verify whether output move events
    bool outputMoveEvents;

    /// matching variables
    /// Match method to be used
    MatchMethod matchMethod;

    /// Available vehicles
    std::set<int> availableVehs;

    /// Booking queue
    std::map<int, Booking> bookingsQueue;

    /// Matching Interval
    double matchingInterval;

    /// Next Matching time
    double nextMatchingTime;

    /// Distance Cost factor
    double distanceCostFactor;

    /// Waiting Time Cost Factor
    double waitingTimeCostFactor;

    /// rebalancing variables
    /// Poiter to demand estimator
    amod::DemandEstimator *demandEstimator;

    /// stations in the world
    std::map<int, amod::Location> stations;

    /// Veh Id to station Id map
    std::unordered_map<int, int> vehIdToStationId;

    /// kdtree of stations for quick lookup
    kdt::KDTree<amod::Location> stationsTree;

    /// use current queue flag
    bool useCurrentQueue;

    /// Rebalancing interval
    double rebalancingInterval;

    /// Next rebalancing time
    double nextRebalancingTime;

    /**
     * demo function to show how to get information from
     * if loc_id is a valid location id, we the waiting customers from that location.
     * if loc_id == 0, then we get all the waiting customers
     * @param worldState amod world
     * @param locId location id
     */
    virtual int getNumWaitingCustomers(amod::World *worldState, int locId = 0);

    /**
     * getClosestStationId
     * returns the closest station id for the given station
     * @param pos Position
     * @return closest station id
     */
    virtual int getClosestStationId(const amod::Position &pos) const;

    /**
     * solveMatching
     * solves the assignment problem and dispatches vehicles to serve bookings
     * @param worldState Pointer to amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode solveMatching(amod::World *worldState);

    /**
     * solveMatchingGreedy
     * solves the assignment problem in a greedy FIFO manner
     * @param worldState Pointer to amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode solveMatchingGreedy(amod::World *worldState);

    /**
     * solveRebalancing
     * solves the rebalancing problem as an LP and dispatches vehicles to other stations.
     * @param worldState Pointer to amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode solveRebalancing(amod::World *worldState);

    /**
     * interStationDispatch
     * sends to_dispatch vehicles from st_source to st_dest
     * @param stSrc source station
     * @param stDest Destination stations
     * @param toDispatch
     * @param worldState Pointer to amod world
     * @param vi
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode interStationDispatch(int stSrc, int stDest,
                                                  int toDispatch,
                                                  amod::World *worldState,
                                                  std::unordered_map<int, std::set<int>> &vi);

    /**
     * updateBookingsFromFile
     * gets bookings that occured during the time period and place it into the bookings_ structure
     * from the last update until the current time
     * @param currTime current time
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode updateBookingsFromFile(double currTime);

    /**
     * isBookingValid
     * performs preliminary checks to ensure the booking is valid
     * @param world pointer to the amod world
     * @param bk reference to the booking to be checked
     * @return true if booking is valid
     */
    virtual bool isBookingValid(amod::World *world, const amod::Booking &bk);
};
}

}
