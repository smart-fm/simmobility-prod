/*
 * Manager.h
 *  Abstract base class for managers to inherit from
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#pragma once

#include "Types.hpp"
#include "Simulator.hpp"
#include "Logger.hpp"
#include "Booking.hpp"
#include <stdexcept>

namespace sim_mob{

namespace amod {

class Manager {
public:
    /**
     * Constructor
     */
    Manager() : simulator(nullptr), logger(nullptr), verbose(false) {}

    /**
     * Destructor
     */
    virtual ~Manager() {}

    /**
     * init
     * initializes the manager with the World world_state
     * @param worldState Pointer to the amod world
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode init(World *worldState) = 0;
    
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
    virtual amod::ReturnCode update(World *worldState) = 0;
    
    /**
     * loadBookings
     * loads bookings that the manager should respond to.
     * @param bookings list of bookings
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode loadBookings(const std::vector<Booking> &bookings) = 0;
    
    /**
     * setSimulator
     * sets the simulator that this manager will use.
     * @param sim - Simulator to be used
     */
    virtual void setSimulator(amod::Simulator *sim) {
        if (!sim) {
            throw std::runtime_error("Manager::setSimulator: sim is nullptr");
        }
	simulator = sim;
    }

    /**
     * getSimulator
     * gets the simulator that this manager will use.
     * @return simulator currently in use
     */
    virtual amod::Simulator* getSimulator() {
	return simulator;
    }
    
    /**
     * setLogger
     * sets the logger for this manager
     * @param logger_ - Logger to be used
     */
    virtual void setLogger(amod::Logger *logger_) {
	if (!logger_) {
            throw std::runtime_error("Manager::setSimulator: logger is nullptr");
        }
	logger = logger_;
    }

    /**
     * getLogger
     * @return logger currently in use
     */
    virtual amod::Logger* getLogger() {
	return logger;
    }
    
    /**
     * setVerbose
     * sets whether debug messages to be printed during simulation
     * @param v flag
     */
    virtual void setVerbose(bool v) {
	verbose = v;
    }
    
    /**
     * getVerbose
     * @return true if verbose is set, else false
     */
    virtual bool getVerbose() {
	return verbose;
    }
    
protected:
    /// Pointer to the simulator
    amod::Simulator* simulator;

    /// Pointer to the logger
    amod::Logger *logger;
    
    /// verbose flag
    bool verbose;

};

}
}
