//
//  ManagerBasic.h
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

#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <ctime>

namespace sim_mob{

namespace amod {
class ManagerBasic : public Manager {
public:
    /**
     * Constructor
     */
    ManagerBasic();

    /**
     * Destructor
     */
    virtual ~ManagerBasic();

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
    virtual amod::ReturnCode loadBookings(const std::vector<Booking> &bookings_);

    /**
     * loadBookingsFromFile
     * loads bookings from a file specified by filename that the manager should respond to.
     * @param filename booking file name
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode loadBookingsFromFile(const std::string& filename);


private:
    /// List of bookings
    std::list<Booking> bookings;

    /// output file
    std::ofstream outFile;

    /// tracks number of available vehicles
    int numAvailVeh;

    /**
     * demo function to show how to get information from
     * if loc_id is a valid location id, we the waiting customers from that location.
     * if loc_id == 0, then we get all the waiting customers
     * @param worldState amod world
     * @param locId location id
     */
    virtual int getNumWaitingCustomers(amod::World *worldState, int locId = 0);
};
}
}

